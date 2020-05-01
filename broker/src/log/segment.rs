use async_trait::async_trait;
use std::path::Path;

use crate::log::record::Record;
use crate::log::result::Result;

pub trait Len {
    fn len(&self) -> u64;

    fn is_empty(&self) -> bool {
        self.len() == 0
    }

    fn is_full(&self) -> bool;
}

#[async_trait]
pub trait Segment: Len {
    async fn open(dir: &Path, path: &str, max_size: u64) -> Self;
    async fn append(&mut self, record: Record) -> Result<u64>;
    async fn lookup(&mut self, offset: u64) -> Result<Record>;
}
