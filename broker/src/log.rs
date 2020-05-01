pub mod filesegment;
pub mod offsets;
pub mod record;
pub mod result;
pub mod segment;

use std::path::Path;

use crate::log::offsets::Offsets;
use crate::log::record::Record;
use crate::log::result::Result;
use crate::log::segment::Segment;

const MAX_SEGMENT_SIZE: u64 = 1_000_000_000;

pub struct Log<T: Segment> {
    offsets: Offsets<T>,
    dir: String,
}

impl<T: Segment> Log<T> {
    pub async fn open(dir: String) -> Log<T> {
        let mut s = Log {
            offsets: Offsets::new(),
            dir: dir.to_string(),
        };
        s.load_segments().await;
        s
    }

    pub async fn append(&mut self, record: Record) -> Result<u64> {
        if self.offsets.active()?.is_full() {
            let segment = T::open(Path::new(&self.dir), "test2", MAX_SEGMENT_SIZE).await;
            self.offsets.add(segment);
        }
        self.offsets.active()?.append(record).await
    }

    pub async fn lookup(&mut self, offset: u64) -> Result<Record> {
        self.offsets.lookup(offset)?.lookup(offset).await
    }

    async fn load_segments(&mut self) {
        // TODO(AD) Must load all segments in dir.
        let segment = T::open(Path::new(&self.dir), "0.seg", MAX_SEGMENT_SIZE).await;
        self.offsets.add(segment);
    }
}
