#![feature(plugin)]
#![plugin(rocket_codegen)]

extern crate rocket;
extern crate serde;
extern crate serde_json;
#[macro_use] extern crate serde_derive;
#[macro_use] extern crate rocket_contrib;

use std::path::{Path, PathBuf};
use std::sync::Mutex;

use rocket::response::NamedFile;
use rocket::State;
use rocket_contrib::{JSON, Value};


mod device;
use device::{DeviceConfig, NetworkConfig, SwitchConfig}; 

type NetworkState = Mutex<NetworkConfig>;

#[get("/api/config")]
fn config(cfg: State<NetworkState>) -> JSON<NetworkConfig> {
    let config = cfg.lock().expect("Network state lock");
    println!("{}", *config);
    JSON(config.clone())
}

#[post("/api/config/<device_id>", format="application/json", data="<dcfg>")]
fn update_device(device_id: u8, dcfg: JSON<DeviceConfig>, cfg: State<NetworkState>) -> JSON<Value> {
    let mut config = cfg.lock().expect("Network state lock");
    *config = config.replace_device(device_id, &dcfg.0);
    JSON(json!({ "status": "ok" }))
}

#[post("/api/config/<device_id>/<switch_id>", format="application/json", data="<scfg>")]
fn update_switch(device_id: u8, switch_id: u8, scfg: JSON<SwitchConfig>, cfg: State<NetworkState>) -> Option<JSON<Value>> {
    let mut config = cfg.lock().expect("Network state lock");
    match config.replace_switch(device_id, switch_id, &scfg.0) {
        Some(v) => *config = v,
        None => return None,
    };
    Some(JSON(json!({ "status": "ok" })))
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

fn main() {
    let network = Mutex::new(NetworkConfig::new());
    rocket::ignite()
        .mount("/", routes![config, update_device, update_switch, index, files])
        .manage(network)
        .launch();
}
