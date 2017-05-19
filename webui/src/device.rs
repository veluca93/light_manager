extern crate serde_json;
use serde_json::Error;

use std::collections::HashMap;
use std::fmt;
use std::fs::File;
use std::path::PathBuf;
use std::io::Read;

#[derive(Serialize, Deserialize, Clone)]
pub struct SwitchConfig {
    name: String,
    buttons: Vec<(u8, u8)>,
    pirs: Vec<(u8, u8)>,
}

#[derive(Serialize, Deserialize, Clone)]
pub struct DeviceConfig {
    name: String,
    switches: HashMap<u8, SwitchConfig>,
    num_buttons: u8,
    num_pirs: u8,
    battery_powered: bool,
}

#[derive(Serialize, Deserialize, Clone)]
pub struct NetworkConfig {
    devices: HashMap<u8, DeviceConfig>,
}

impl fmt::Display for NetworkConfig {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        let res = serde_json::to_string(self).unwrap(); // TODO: improve this
        fmt.write_str(&res)
    }
}

impl DeviceConfig {
    fn replace_switch(&self, id: u8, scfg: &SwitchConfig) -> DeviceConfig {
        let mut cfg = DeviceConfig {
            switches: HashMap::new(),
            name: self.name.clone(),
            num_buttons: self.num_buttons,
            num_pirs: self.num_pirs,
            battery_powered: self.battery_powered,
        };
        for switch in self.switches.iter() {
            cfg.switches.insert(
                *switch.0,
                if *switch.0 == id {
                    (*scfg).clone()
                } else {
                    (*switch.1).clone()
                }
            );
        };
        cfg
    }
}

impl NetworkConfig {
    pub fn new() -> NetworkConfig {
        NetworkConfig { devices: HashMap::new() }
    }

    pub fn parse_from_file(path: PathBuf) -> Result<NetworkConfig, Error> {
        let mut data = String::new();
        match try!(File::open(path)).read_to_string(&mut data) {
            Err(e) => return Err(Error::from(e)),
            Ok(_) => (),
        }
        let res: NetworkConfig = try!(serde_json::from_str(&data));
        Ok(res)
    }

    pub fn replace_switch(&self, device_id: u8, switch_id: u8, scfg: &SwitchConfig) -> Option<NetworkConfig> {
        if !self.devices.contains_key(&device_id) {
            return None
        }
        let mut cfg = NetworkConfig { devices: HashMap::new()};
        for device in self.devices.iter() {
            cfg.devices.insert(
                *device.0,
                if *device.0 == device_id {
                    (*device.1).replace_switch(switch_id, scfg)
                } else {
                    (*device.1).clone()
                }
            );
        };
        Some(cfg)
    }

    pub fn replace_device(&self, id: u8, dcfg: &DeviceConfig) -> NetworkConfig {
        let mut cfg = NetworkConfig { devices: HashMap::new()};
        for device in self.devices.iter() {
            cfg.devices.insert(
                *device.0,
                if *device.0 == id {
                    (*dcfg).clone()
                } else {
                    (*device.1).clone()
                }
            );
        };
        if !cfg.devices.contains_key(&id) {
            cfg.devices.insert(id, (*dcfg).clone());
        }
        cfg
    }
}
