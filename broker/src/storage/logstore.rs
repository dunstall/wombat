use std::collections::HashMap;

use crate::storage::log::Log;
use crate::storage::segment::{LogResult, Segment};

// Maximum segment size of 1GB.
const MAX_SEGMENT_SIZE: u64 = 1_000_000_000;

pub struct LogStore<T: Segment> {
    active_offset: u64,
    offsets: HashMap<u64, T>,
}

impl<T: Segment> LogStore<T> {
    pub fn new(_dirs: &str) -> LogStore<T> {
        // TODO load current segments in dirs.

        let segment = T::open("test");
        let mut offsets = HashMap::new();
        offsets.insert(0, segment);
        LogStore {
            active_offset: 0,
            offsets: offsets,
        }
    }

    pub fn append(&mut self, log: Log) -> LogResult<u64> {
        if self.active_expired() {
            self.update_active();
        }
        self.active().append(log)
    }

    pub fn lookup(&mut self, offset: u64) -> LogResult<Log> {
        // TODO for now just use active only
        self.active().lookup(offset)
    }

    fn update_active(&mut self) {
        self.active_offset += self.active_size();
        let segment = T::open("test2");
        self.offsets.insert(self.active_offset, segment);
    }

    fn active_expired(&mut self) -> bool {
        self.active_size() > MAX_SEGMENT_SIZE
    }

    fn active_size(&mut self) -> u64 {
        // self.active().size()
        0
    }

    fn active(&mut self) -> &mut T {
        match self.offsets.get_mut(&self.active_offset) {
            // This is a non-recoverable error so just crash.
            None => panic!("active segment does not exist"),
            Some(active) => active,
        }
    }
}
