use std::io;

use wombatlog::LogError;

#[derive(std::fmt::Debug)]
pub enum PartitionError {
    LogError(LogError),
    IOError(io::Error),
}

impl From<io::Error> for PartitionError {
    fn from(error: io::Error) -> Self {
        PartitionError::IOError(error)
    }
}

impl From<LogError> for PartitionError {
    fn from(error: LogError) -> Self {
        PartitionError::LogError(error)
    }
}

pub type PartitionResult<T> = std::result::Result<T, PartitionError>;
