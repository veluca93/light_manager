
use std::path::PathBuf;
use std::fs::DirBuilder;
use std::fs::File;
use std::fs::rename;
use std::thread::{self, JoinHandle};
use std::sync::mpsc::{channel, Sender};
use std::sync::{Arc, Mutex};

use std::io::Write;

pub struct Keeper {
    path: PathBuf,
    filename: String,
    thread: Option<JoinHandle<()>>,
    tx: Option<Sender<String>>,
}

impl Keeper {
    pub fn new(path: PathBuf, filename: &str) -> Keeper {
        DirBuilder::new()
            .recursive(true)
            .create(&path).unwrap();
        Keeper {
            path: path,
            filename: filename.to_string(),
            thread: None,
            tx: None,
        }
    }

    pub fn start(mut self) -> Arc<Mutex<Keeper>> {
        let (tx, rx) = channel();
        self.tx = Some(tx);
        let ret = Arc::new(Mutex::new(self));
        let locked_self = ret.clone();
        ret.lock().unwrap().thread = Some(thread::spawn(move ||
            loop {
                let data = rx.recv().unwrap();
                let s = locked_self.lock().unwrap();
                match File::create(&s.path.join("_tmp.".to_string() + &s.filename))
                        .map(|mut file| file.write_all(data.as_bytes())) {
                    Ok(_) => match rename(
                                &s.path.join("_tmp.".to_string() + &s.filename),
                                &s.path.join(&s.filename)
                            ) {
                        Ok(_) => (),
                        Err(why) => println!("Error moving file: {}", why),
                    },
                    Err(why) => println!("Error writing file: {}", why),
                }
            }
        ));
        ret
    }

    pub fn send(&mut self, data: &str) {
        self.tx.as_ref().unwrap().send(data.to_string()).unwrap();
    }
}
