use crate::log::log::Log;
use crate::log::result::Result;

pub trait Segment {
    fn open(dir: &str, path: &str) -> Self;
    fn append(&mut self, log: Log) -> Result<u64>;
    fn lookup(&mut self, offset: u64) -> Result<Log>;
    fn size(&mut self) -> Result<u64>;
}
