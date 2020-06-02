use wombatlog::LogError;

#[derive(std::fmt::Debug)]
pub enum PartitionError {
    LogError(LogError),
}

impl From<LogError> for PartitionError {
    fn from(error: LogError) -> Self {
        PartitionError::LogError(error)
    }
}

pub type PartitionResult<T> = std::result::Result<T, PartitionError>;
