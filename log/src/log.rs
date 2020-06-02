use std::time::SystemTime;
use std::vec::Vec;

use crate::result::LogResult;

pub trait Log {
    fn append(&mut self, data: &Vec<u8>) -> LogResult<()>;

    fn lookup(&mut self, size: u64, offset: u64) -> LogResult<Vec<u8>>;

    fn expire(&mut self, before: SystemTime) -> LogResult<()>;
}
