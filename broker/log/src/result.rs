use std::ffi;
use std::io;
use std::string;

#[derive(std::fmt::Debug)]
pub enum LogError {
    OffsetNotFound,
    SegmentExpired,
    IoError(io::Error),
    Utf8Error(string::FromUtf8Error),
    OsStringError(ffi::OsString),
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

impl From<ffi::OsString> for LogError {
    fn from(error: ffi::OsString) -> Self {
        LogError::OsStringError(error)
    }
}

pub type LogResult<T> = std::result::Result<T, LogError>;
