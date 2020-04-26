use crate::log::log::Log;
use crate::log::result::LogResult;

pub trait Segment {
    fn open(dir: &str, path: &str) -> Self;
    fn append(&mut self, log: Log) -> LogResult<u64>;
    fn lookup(&mut self, offset: u64) -> LogResult<Log>;
    fn size(&mut self) -> LogResult<u64>;
}
