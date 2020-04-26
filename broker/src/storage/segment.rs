use std::io;

use crate::storage::log::Log;

#[derive(Debug)]
pub enum LogError {
    IoError(io::Error),
    DecodeError(&'static str),
    LogCorrupted,
}

pub type LogResult<T> = std::result::Result<T, LogError>;

impl From<io::Error> for LogError {
    fn from(error: io::Error) -> Self {
        LogError::IoError(error)
    }
}

pub trait Segment {
    fn open(dir: &str, path: &str) -> Self;
    fn append(&mut self, log: Log) -> LogResult<u64>;
    fn lookup(&mut self, offset: u64) -> LogResult<Log>;
    fn size(&mut self) -> LogResult<u64>;
}
