use crate::log::log::Log;
use crate::log::result::Result;

use async_trait::async_trait;

pub trait Len {
    fn len(&self) -> u64;

    fn is_empty(&self) -> bool {
        self.len() == 0
    }

    fn is_full(&self) -> bool {
        false // TODO
    }
}

#[async_trait]
pub trait Segment: Len {
    async fn open(dir: &str, path: &str) -> Self;
    async fn append(&mut self, log: Log) -> Result<u64>;
    async fn lookup(&mut self, offset: u64) -> Result<Log>;
}
