use std::io;

#[derive(Debug)]
pub enum Error {
    IoError(io::Error),
    RecordCorrupted,
    OffsetNotFound,
}

pub type Result<T> = std::result::Result<T, Error>;

impl From<io::Error> for Error {
    fn from(error: io::Error) -> Self {
        Error::IoError(error)
    }
}
