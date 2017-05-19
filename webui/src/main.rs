#![feature(plugin)]
#![plugin(rocket_codegen)]

extern crate rocket;
extern crate serde;
extern crate serde_json;
#[macro_use] extern crate serde_derive;
#[macro_use] extern crate rocket_contrib;

use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

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

type NetworkState = Mutex<NetworkConfig>;
type KeeperSend = Arc<Mutex<Keeper>>;
type DB = Arc<Mutex<EventManager>>;

#[get("/api/config")]
fn config(cfg: State<NetworkState>) -> JSON<NetworkConfig> {
    let config = cfg.lock().expect("Network state lock");
    JSON(config.clone())
}

#[post("/api/config/<device_id>", format="application/json", data="<dcfg>")]
fn update_device(device_id: u8, dcfg: JSON<DeviceConfig>, cfg: State<NetworkState>, keeper: State<KeeperSend>) -> JSON<Value> {
    let mut config = cfg.lock().expect("Network state lock");
    *config = config.replace_device(device_id, &dcfg.0);
    keeper.lock().unwrap().send(&config.to_string());
    JSON(json!({ "status": 200 }))
}

#[post("/api/config/<device_id>/<switch_id>", format="application/json", data="<scfg>")]
fn update_switch(device_id: u8, switch_id: u8, scfg: JSON<SwitchConfig>, cfg: State<NetworkState>, keeper: State<KeeperSend>) -> Option<JSON<Value>> {
    let mut config = cfg.lock().expect("Network state lock");
    match config.replace_switch(device_id, switch_id, &scfg.0) {
        Some(v) => *config = v,
        None => return None,
    };
    keeper.lock().unwrap().send(&config.to_string());
    Some(JSON(json!({ "status": 200 })))
}

#[derive(Serialize, Deserialize)]
struct TimestampInterval {
    start: i64,
    stop: i64,
}

#[post("/api/events/<id>", format="application/json", data="<interval>")]
fn get_events(id: u8, interval: JSON<TimestampInterval>, db: State<DB>) -> Option<JSON<Vec<Event>>> {
    let result = db.lock().unwrap().query(id, interval.0.start, interval.0.stop);
    if result.len() == 0 {
        None
    } else {
        Some(JSON(result))
    }
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

fn main() {
    let webserver = rocket::ignite()
        .mount("/", routes![config, update_device, update_switch, get_events, index, files])
        .catch(errors![not_found]);
    const DEFAULT_STORAGE_DIR: &'static str = "data";
    let data_dir = PathBuf::new().join(
        config::active()
        .and_then(
            |config| match config.extras.get("storage_dir") {
                Some(v) => v.as_str(),
                None => None,
            }).unwrap_or_else(|| DEFAULT_STORAGE_DIR)
    );

    let network = Mutex::new(
        NetworkConfig::parse_from_file(data_dir.join("network.json"))
        .unwrap_or(NetworkConfig::new())
    );

    let keeper = Keeper::new(data_dir.clone(), "network.json").start();

    let db = Arc::new(Mutex::new(EventManager::connect(data_dir, "events.sqlite")));

    webserver.manage(network).manage(keeper).manage(db).launch();
}
