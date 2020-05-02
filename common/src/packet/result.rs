use std::io;

#[derive(Debug)]
pub enum MessageError {
    IoError(io::Error),
    DecodeError(&'static str),
}

pub type MessageResult<T> = std::result::Result<T, MessageError>;

impl From<io::Error> for MessageError {
    fn from(error: io::Error) -> Self {
        MessageError::IoError(error)
    }
}
