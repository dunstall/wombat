#[derive(std::fmt::Debug)]
pub enum ServerError {
    IoError(tokio::io::Error),
}

impl From<tokio::io::Error> for ServerError {
    fn from(error: tokio::io::Error) -> Self {
        ServerError::IoError(error)
    }
}

pub type ServerResult<T> = std::result::Result<T, ServerError>;
