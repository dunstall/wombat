use std::io;

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
