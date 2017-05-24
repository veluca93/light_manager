#![feature(plugin)]
#![plugin(rocket_codegen)]

extern crate rocket;
extern crate serde;
extern crate serde_json;
extern crate serial;
#[macro_use] extern crate serde_derive;
#[macro_use] extern crate rocket_contrib;

use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

use std::thread;
use std::time;

use rocket::response::NamedFile;
use rocket::State;
use rocket_contrib::{JSON, Value};
use rocket::config;

mod device;
use device::{DeviceConfig, NetworkConfig, SwitchConfig};

mod keeper;
use keeper::Keeper;

mod events;
use events::{Event, EventManager};

mod comm;
use comm::SerialComm;

type NetworkState = Mutex<NetworkConfig>;
type KeeperSend = Arc<Mutex<Keeper>>;
type DB = Arc<Mutex<EventManager>>;
type Serial = Arc<Mutex<SerialComm>>;

#[get("/api/config")]
fn config(cfg: State<NetworkState>) -> JSON<NetworkConfig> {
    let config = cfg.lock().expect("Network state lock");
    JSON(config.clone())
}

#[post("/api/config/<device_id>", format="application/json", data="<dcfg>")]
fn update_device(
    device_id: u8, dcfg: JSON<DeviceConfig>, cfg: State<NetworkState>,
    keeper: State<KeeperSend>, serial: State<Serial>) -> JSON<Value>
{
    let mut config = cfg.lock().expect("Network state lock");
    *config = config.replace_device(device_id, &dcfg.0);
    for switch in dcfg.switches.iter() {
        for i in config.devices.iter() {
            serial.lock().expect("Serial lock").update_switch(device_id, *switch.0, switch.1, *i.0);
            thread::sleep(time::Duration::from_millis(10)); // Avoid flooding the network
        }
        serial.lock().expect("Serial lock").update_switch_pir_time(device_id, *switch.0, switch.1);
    }
    keeper.lock().unwrap().send(&config.to_string());
    JSON(json!({ "status": 200 }))
}

#[delete("/api/config/<device_id>")]
fn delete_device(device_id: u8, cfg: State<NetworkState>, keeper: State<KeeperSend>) -> Option<JSON<Value>> {
    let mut config = cfg.lock().expect("Network state lock");
    if config.devices.contains_key(&device_id) {
        config.devices.remove(&device_id);
        keeper.lock().unwrap().send(&config.to_string());
        Some(JSON(json!({ "status": 200 })))
    } else {
        None
    }
}

#[post("/api/config/<device_id>/<switch_id>", format="application/json", data="<scfg>")]
fn update_switch(
    device_id: u8, switch_id: u8, scfg: JSON<SwitchConfig>, cfg: State<NetworkState>,
    keeper: State<KeeperSend>, serial: State<Serial>) -> Option<JSON<Value>>
{
    let mut config = cfg.lock().expect("Network state lock");
    if config.devices.contains_key(&device_id) {
        for i in config.devices.iter() {
            serial.lock().expect("Serial lock").update_switch(device_id, switch_id, &scfg.0, *i.0);
            thread::sleep(time::Duration::from_millis(10)); // Avoid flooding the network
        }
        serial.lock().expect("Serial lock").update_switch_pir_time(device_id, switch_id, &scfg.0);
        match config.replace_switch(device_id, switch_id, &scfg.0) {
            Some(v) => *config = v,
            None => return None,
        };
    } else {
        return None;
    }
    keeper.lock().unwrap().send(&config.to_string());
    Some(JSON(json!({ "status": 200 })))
}

#[post("/api/turn_on/<device_id>/<switch_id>")]
fn turn_on(device_id: u8, switch_id: u8, cfg: State<NetworkState>, serial: State<Serial>) -> Option<JSON<Value>> {
    let config = cfg.lock().expect("Network state lock");
    if config.devices.contains_key(&device_id) {
        let device = config.devices.get(&device_id).unwrap();
        if device.switches.contains_key(&switch_id) {
            serial.lock().unwrap().send_command(device_id, switch_id, true);
            Some(JSON(json!({ "status": 200 })))
        } else {
            None
        }
    } else {
        None
    }
}

#[post("/api/turn_off/<device_id>/<switch_id>")]
fn turn_off(device_id: u8, switch_id: u8, cfg: State<NetworkState>, serial: State<Serial>) -> Option<JSON<Value>> {
    let config = cfg.lock().expect("Network state lock");
    if config.devices.contains_key(&device_id) {
        let device = config.devices.get(&device_id).unwrap();
        if device.switches.contains_key(&switch_id) {
            serial.lock().unwrap().send_command(device_id, switch_id, false);
            Some(JSON(json!({ "status": 200 })))
        } else {
            None
        }
    } else {
        None
    }
}

#[derive(Serialize, Deserialize)]
struct TimestampInterval {
    start: i64,
    stop: i64,
}

#[post("/api/events", format="application/json", data="<interval>")]
fn get_events(interval: JSON<TimestampInterval>, db: State<DB>) -> JSON<Vec<Event>> {
    let result = db.lock().unwrap().query(interval.0.start, interval.0.stop);
    JSON(result)
}

#[get("/")]
fn index() -> Option<NamedFile> {
    let path = PathBuf::new().join("index.html");
    files(path)
}

#[get("/<file..>")]
fn files(file: PathBuf) -> Option<NamedFile> {
    NamedFile::open(Path::new("static/").join(file)).ok()
}

#[error(404)]
fn not_found() -> JSON<Value> {
    JSON(json!({ "status": 404, "msg": "Not found"}))
}

#[error(500)]
fn internal_server_error() -> JSON<Value> {
    JSON(json!({ "status": 500, "msg": "Internal server error"}))
}

#[error(400)]
fn bad_request() -> JSON<Value> {
    JSON(json!({ "status": 400, "msg": "Bad request"}))
}

fn main() {
    let webserver = rocket::ignite()
        .mount("/", routes![
            update_device, update_switch, turn_on, turn_off, delete_device,
            get_events, config, index, files 
        ]).catch(errors![not_found, internal_server_error, bad_request]);
    const DEFAULT_STORAGE_DIR: &'static str = "data";
    let data_dir = PathBuf::new().join(
        config::active()
        .and_then(
            |config| match config.extras.get("storage_dir") {
                Some(v) => v.as_str(),
                None => None,
            }).unwrap_or_else(|| DEFAULT_STORAGE_DIR)
    );

    const DEFAULT_SERIAL_PORT: &'static str = "/dev/ttyACM0";
    let serial_port = config::active()
        .and_then(
            |config| match config.extras.get("serial_port") {
                Some(v) => v.as_str(),
                None => None,
            }).unwrap_or_else(|| DEFAULT_SERIAL_PORT);

    let network = Mutex::new(
        NetworkConfig::parse_from_file(data_dir.join("network.json"))
        .unwrap_or(NetworkConfig::new())
    );

    let keeper = Keeper::new(data_dir.clone(), "network.json").start();

    let db = Arc::new(Mutex::new(EventManager::connect(data_dir, "events.sqlite")));

    let serial_port = SerialComm::open(serial_port, db.clone()).start();

    webserver.manage(network).manage(keeper).manage(db).manage(serial_port).launch();
}
