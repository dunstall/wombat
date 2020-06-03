use std::time::SystemTime;
use std::vec::Vec;

use crate::log::Log;
use crate::result::{LogError, LogResult};

/// In memory log for testing other components.
pub struct InMemoryLog {
    data: Vec<u8>,
}

impl InMemoryLog {
    pub fn new() -> InMemoryLog {
        InMemoryLog { data: Vec::new() }
    }
}

impl Log for InMemoryLog {
    fn append(&mut self, data: &Vec<u8>) -> LogResult<()> {
        self.data.append(&mut data.clone());
        Ok(())
    }

    fn lookup(&mut self, size: u64, offset: u64) -> LogResult<Vec<u8>> {
        if size + offset > self.data.len() as u64 {
            Err(LogError::Eof)
        } else {
            Ok(self.data[offset as usize..(size + offset) as usize].to_vec())
        }
    }

    fn expire(&mut self, _before: SystemTime) -> LogResult<()> {
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn append_and_lookup() {
        let mut log = InMemoryLog::new();

        let written = vec![1, 2, 3];
        log.append(&written).unwrap();

        let read = log.lookup(3, 0).unwrap();
        assert_eq!(written, read);
    }

    #[test]
    fn lookup_eof() {
        let mut log = InMemoryLog::new();
        if let Err(LogError::Eof) = log.lookup(4, 0) {
        } else {
            panic!("expected segment expired");
        }
    }
}
