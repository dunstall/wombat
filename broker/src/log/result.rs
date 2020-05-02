use std::io;

#[derive(std::fmt::Debug)]
pub enum LogError {
    IoError(io::Error),
}

impl From<io::Error> for LogError {
    fn from(error: io::Error) -> Self {
        LogError::IoError(error)
    }
}

pub type LogResult<T> = std::result::Result<T, LogError>;
