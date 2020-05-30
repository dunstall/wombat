use std::io;
use std::string;

#[derive(std::fmt::Debug)]
pub enum LogError {
    IoError(io::Error),
    Utf8Error(string::FromUtf8Error),
}

impl From<io::Error> for LogError {
    fn from(error: io::Error) -> Self {
        LogError::IoError(error)
    }
}

impl From<string::FromUtf8Error> for LogError {
    fn from(error: string::FromUtf8Error) -> Self {
        LogError::Utf8Error(error)
    }
}

pub type LogResult<T> = std::result::Result<T, LogError>;
