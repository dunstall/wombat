use std::cmp::PartialEq;
use std::io::Cursor;
use std::vec::Vec;

use crate::storage::logheader::LogHeader;
use crate::storage::segment::{LogError, LogResult};

pub struct Log {
    header: LogHeader,
    key: Vec<u8>,
    val: Vec<u8>,
}

// TODO unit test
impl Log {
    pub fn new(header: LogHeader, key: Vec<u8>, val: Vec<u8>) -> Log {
        Log { header, key, val }
    }

    pub fn encode(&mut self) -> LogResult<Vec<u8>> {
        let mut enc = self.header.encode()?;
        enc.append(&mut self.key);
        enc.append(&mut self.val);
        Ok(enc)
    }

    pub fn verify_crc(&self) -> LogResult<()> {
        Err(LogError::LogCorrupted)
    }
}
