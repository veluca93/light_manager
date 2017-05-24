
use serial::prelude::*;
use serial::{self, SystemPort};
use events::{EventManager, Event, EventType};
use device::SwitchConfig;
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use std::time::Duration;

use std::io::{Read, Write};

pub struct SerialComm {
    port: SystemPort,
    db: Arc<Mutex<EventManager>>,
    thread: Option<JoinHandle<()>>,
}

impl SerialComm {
    pub fn open(path: &str, db: Arc<Mutex<EventManager>>) -> SerialComm {
        let mut port = serial::open(path).unwrap();
        port.reconfigure(&|settings| {
            try!(settings.set_baud_rate(serial::Baud9600));
            settings.set_char_size(serial::Bits8);
            settings.set_parity(serial::ParityNone);
            settings.set_stop_bits(serial::Stop1);
            settings.set_flow_control(serial::FlowNone);
            Ok(())
        }).unwrap();
        port.set_timeout(Duration::from_millis(10)).unwrap();
        SerialComm {port: port, db: db, thread: None}
    }

    pub fn start(self) -> Arc<Mutex<SerialComm>> {
        let ret = Arc::new(Mutex::new(self));
        let locked_self = ret.clone();
        ret.lock().unwrap().thread = Some(thread::spawn(move || {
            loop {
                thread::sleep(Duration::from_millis(10));
                let mut s = locked_self.lock().unwrap();
                let mut buffer = [0; 6];
                match s.port.read_exact(&mut buffer) {
                    Ok(_) => (),
                    Err(_) => continue
                }
                if buffer[0] != 123 {
                    println!("Received garbage");
                    continue;
                }
                let node_id = buffer[2];
                let bat_lvl = buffer[3];
                match buffer[1] {
                    0 => {
                        let is_pir = buffer[4];
                        let id = buffer[5];
                        let event = Event::new(
                            node_id,
                            if is_pir == 1 { EventType::PIRIsOn(id) } else {EventType::ButtonPress(id)},
                            bat_lvl
                        );
                        s.db.lock().unwrap().insert(&event);
                    },
                    1 => {
                        let switch_status = buffer[4];
                        for i in 0..4 {
                            let event = Event::new(
                                node_id,
                                if switch_status & (1<<i) != 0 {EventType::SwitchIsOn(i)} else {EventType::SwitchIsOff(i)},
                                bat_lvl
                            );
                            s.db.lock().unwrap().insert(&event);
                        }
                    },
                    _ => {
                        println!("Received garbage");
                        continue;
                    },
                }

            }
        }));
        ret
    }

    pub fn update_switch(&mut self, device_id: u8, switch_id: u8, scfg: &SwitchConfig, source: u8) {
        let mut data = [0; 7];
        data[0] = 123;
        data[1] = 1;
        data[2] = device_id;
        data[3] = switch_id;
        data[4] = source;
        // Button configuration
        let mut bitmask = 0;
        for button in &scfg.buttons {
            if button.0 == source {
                bitmask |= 1<<button.1;
            }
        }
        println!("{}", bitmask);
        data[5] = bitmask;
        data[6] = 0;
        self.port.write_all(&data).unwrap();
        // PIR configuration
        let mut bitmask = 0;
        for pir in &scfg.pirs {
            if pir.0 == source {
                bitmask |= 1<<pir.1;
            }
        }
        data[5] = bitmask;
        data[6] = 1;
        self.port.write_all(&data).unwrap();
    }

    pub fn update_switch_pir_time(&mut self, device_id: u8, switch_id: u8, scfg: &SwitchConfig) {
        let mut data = [0; 7];
        data[0] = 123;
        data[1] = 1;
        data[2] = device_id;
        data[3] = switch_id;
        data[4] = (scfg.pir_time>>8) as u8;
        data[5] = (scfg.pir_time & 0xFF) as u8;
        self.port.write_all(&data).unwrap();
    }

    pub fn send_command(&mut self, device_id: u8, switch_id: u8, turn_on: bool) {
        let mut data = [0; 7];
        data[0] = 123;
        data[1] = 0;
        data[2] = device_id;
        data[3] = switch_id;
        data[4] = if turn_on {1} else {0};
        self.port.write_all(&data).unwrap();
    }
}
