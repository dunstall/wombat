use std::vec::Vec;

use crate::log::header::Header;
use crate::log::result::{Error, Result};

#[derive(Debug)]
pub struct Log {
    pub header: Header,
    pub key: Vec<u8>,
    pub val: Vec<u8>,
}

// TODO unit test
impl Log {
    pub fn new(header: Header, key: Vec<u8>, val: Vec<u8>) -> Log {
        Log { header, key, val }
    }

    pub fn encode(&mut self) -> Result<Vec<u8>> {
        let mut enc = self.header.encode()?;
        enc.append(&mut self.key);
        enc.append(&mut self.val);
        Ok(enc)
    }

    pub fn verify_crc(&self) -> Result<()> {
        Err(Error::LogCorrupted)
    }
}
