use std::vec::Vec;

use crate::result::LogResult;

pub trait Log {
    fn append(&self, data: &Vec<u8>) -> LogResult<()>;
    fn lookup(&self, size: usize, offset: usize) -> LogResult<Vec<u8>>;
}
