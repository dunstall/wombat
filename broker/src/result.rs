use wombatlog::LogError;
use wombatpartition::PartitionError;

#[derive(std::fmt::Debug)]
pub enum BrokerError {
    LogError(LogError),
    PartitionError(PartitionError),
}

impl From<LogError> for BrokerError {
    fn from(error: LogError) -> Self {
        BrokerError::LogError(error)
    }
}

impl From<PartitionError> for BrokerError {
    fn from(error: PartitionError) -> Self {
        BrokerError::PartitionError(error)
    }
}

pub type BrokerResult<T> = std::result::Result<T, BrokerError>;
