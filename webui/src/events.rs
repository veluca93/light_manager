extern crate rusqlite;

use std::path::PathBuf;

#[derive(Serialize, Deserialize)]
pub enum EventType {
    SwitchIsOn(u8),
    SwitchIsOff(u8),
    ButtonPress(u8),
    PIRIsOn(u8),
    None
}

#[derive(Serialize, Deserialize)]
pub struct Event {
    node_id: u8,
    kind: EventType,
    battery_level: u8,
    date_received: i64
}

pub struct EventManager {
   conn: rusqlite::Connection,  
}

impl Event {
    pub fn new(node_id: u8, kind: EventType, battery_level: u8) -> Event {
        Event {
            node_id: node_id,
            kind: kind,
            battery_level: battery_level,
            date_received: -1,
        }
    }
}

impl EventType {
    pub fn to_pair(&self) -> (u8, Option<u8>) {
        match *self {
            EventType::SwitchIsOn(x) => (1, Some(x)),
            EventType::SwitchIsOff(x) => (2, Some(x)),
            EventType::ButtonPress(x) => (3, Some(x)),
            EventType::PIRIsOn(x) => (4, Some(x)),
            EventType::None => (0, None),
        }
    }

    pub fn from_pair(kind: u8, value: Option<u8>) -> Option<EventType> {
        match (kind, value) {
            (0, _) => Some(EventType::None),
            (1, Some(v)) => Some(EventType::SwitchIsOn(v)),
            (2, Some(v)) => Some(EventType::SwitchIsOff(v)),
            (3, Some(v)) => Some(EventType::ButtonPress(v)),
            (4, Some(v)) => Some(EventType::PIRIsOn(v)),
            _ => None
        }
    }
}


impl EventManager {
    pub fn connect(path: PathBuf, filename: &str) -> EventManager {
        let conn = rusqlite::Connection::open(path.join(filename)).unwrap();
        conn.execute_batch("
            PRAGMA JOURNAL_MODE = WAL;
            PRAGMA SYNCHRONOUS = NORMAL;
            BEGIN;
            CREATE TABLE IF NOT EXISTS events (
                node_id INTEGER NOT NULL,
                kind INTEGER NOT NULL,
                value INTEGER,
                battery_level INTEGER NOT NULL,
                date INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
                CHECK (value IS NOT NULL OR kind == 0),
                CHECK (kind >= 0),
                CHECK (kind < 5)
            );
            CREATE INDEX IF NOT EXISTS id_date_idx ON events(date);
            COMMIT;
        ").unwrap();
            
        EventManager { conn: conn }
    }

    pub fn insert(&self, event: &Event) {
        let mut stmt = self.conn.prepare_cached("
            INSERT INTO events(node_id, kind, value, battery_level)
            VALUES (:node_id, :kind, :value, :battery_level);
        ").unwrap();
        let (kind, value) = event.kind.to_pair();
        stmt.execute_named(&[
            (":node_id", &event.node_id),
            (":kind", &kind),
            (":value", &value),
            (":battery_level", &event.battery_level)
        ]).unwrap();
    }

    pub fn query(&self, start: i64, stop: i64) -> Vec<Event> {
        let mut stmt = self.conn.prepare_cached("
            SELECT node_id, kind, value, battery_level, date
            FROM events
            WHERE date >= :start AND date <= :stop
            ORDER BY date ASC;
        ").unwrap();
        let events = stmt.query_map_named(&[
            (":start", &start),
            (":stop", &stop)],
            |row| Event {
                node_id: row.get(0),
                kind: EventType::from_pair(row.get(1), row.get(2)).unwrap(),
                battery_level: row.get(3),
                date_received: row.get(4),
            }
        ).unwrap();
        let mut res = Vec::new();
        for event in events {
            res.push(event.unwrap());
        }
        res
    }
}
