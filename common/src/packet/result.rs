use std::io;
use std::string;

#[derive(Debug)]
pub enum MessageError {
    IoError(io::Error),
    DecodeError(&'static str),
    Utf8Error(string::FromUtf8Error),
}

pub type MessageResult<T> = std::result::Result<T, MessageError>;

impl From<io::Error> for MessageError {
    fn from(error: io::Error) -> Self {
        MessageError::IoError(error)
    }
}

impl From<string::FromUtf8Error> for MessageError {
    fn from(error: string::FromUtf8Error) -> Self {
        MessageError::Utf8Error(error)
    }
}
