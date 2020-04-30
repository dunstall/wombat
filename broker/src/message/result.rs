#[derive(Debug)]
pub enum Error {
    DecodeError(&'static str),
}

pub type Result<T> = std::result::Result<T, Error>;
