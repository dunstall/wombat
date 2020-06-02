#[derive(std::fmt::Debug)]
pub enum BrokerError {}

pub type BrokerResult<T> = std::result::Result<T, BrokerError>;
