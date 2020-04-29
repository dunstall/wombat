use std::collections::HashMap;
use std::string::String;

use crate::log::log::Log;
use crate::log::result::Result;
use crate::log::segment::Segment;

// Maximum segment size of 1GB.
const MAX_SEGMENT_SIZE: u64 = 1_000_000_000;

// TODO test with in memory segment

pub struct Store<T: Segment> {
    active_offset: u64,
    offsets: HashMap<u64, T>,
    dir: String,
}

impl<T: Segment> Store<T> {
    pub async fn new(dir: &str) -> Store<T> {
        let mut s = Store {
            active_offset: 0,
            offsets: HashMap::new(),
            dir: dir.to_string(),
        };
        s.load_segments().await;
        s
    }

    pub async fn append(&mut self, log: Log) -> Result<u64> {
        if self.active_expired() {
            self.update_active().await;
        }
        self.active().append(log).await
    }

    pub async fn lookup(&mut self, offset: u64) -> Result<Log> {
        // TODO for now just use active only
        self.active().lookup(offset).await
    }

    async fn update_active(&mut self) {
        self.active_offset += self.active_size();
        let segment = T::open("TEST1", "test2").await;
        self.offsets.insert(self.active_offset, segment);
    }

    async fn load_segments(&mut self) {
        let segment = T::open(&self.dir, "0.seg").await;
        self.offsets.insert(0, segment);
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

#[cfg(test)]
mod tests {
    #[test]
    fn todo() {
        // TODO
    }
}
