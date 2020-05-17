use wombatcore::MessageError;

#[derive(std::fmt::Debug)]
pub enum WombatError {
    IoError(tokio::io::Error),
    MessageError(MessageError),
}

impl From<tokio::io::Error> for WombatError {
    fn from(error: tokio::io::Error) -> Self {
        WombatError::IoError(error)
    }
}

impl From<MessageError> for WombatError {
    fn from(error: MessageError) -> Self {
        WombatError::MessageError(error)
    }
}

pub type WombatResult<T> = std::result::Result<T, WombatError>;
