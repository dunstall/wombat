use std::string::String;

use crate::log::record::Record;
use crate::log::offsets::Offsets;
use crate::log::result::Result;
use crate::log::segment::Segment;

pub struct Store<T: Segment> {
    offsets: Offsets<T>,
    dir: String,
}

impl<T: Segment> Store<T> {
    pub async fn open(dir: &str) -> Store<T> {
        let mut s = Store {
            offsets: Offsets::new(),
            dir: dir.to_string(),
        };
        s.load_segments().await;
        s
    }

    pub async fn append(&mut self, record: Record) -> Result<u64> {
        if self.offsets.active()?.is_full() {
            let segment = T::open(&self.dir, "test2").await;
            self.offsets.add(segment);
        }
        self.offsets.active()?.append(record).await
    }

    pub async fn lookup(&mut self, offset: u64) -> Result<Record> {
        self.offsets.lookup(offset)?.lookup(offset).await
    }

    async fn load_segments(&mut self) {
        // TODO(AD) Must load all segments in dir.
        let segment = T::open(&self.dir, "0.seg").await;
        self.offsets.add(segment);
    }
}
