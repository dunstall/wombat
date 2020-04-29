use crate::log::log::Log;
use crate::log::result::Result;

use async_trait::async_trait;

#[async_trait]
pub trait Segment {
    async fn open(dir: &str, path: &str) -> Self;
    async fn append(&mut self, log: Log) -> Result<u64>;
    async fn lookup(&mut self, offset: u64) -> Result<Log>;
    async fn size(&mut self) -> Result<u64>;
}
