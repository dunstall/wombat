use std::fs::ReadDir;
use std::path::Path;
use std::time::SystemTime;
use std::vec::Vec;

use crate::segment::Segment;
use crate::result::{LogError, LogResult};

/// In memory segment for testing other components.
pub struct InMemorySegment {
    data: Vec<u8>,
}

impl Segment for InMemorySegment {
    fn open(_path: &Path) -> LogResult<Box<InMemorySegment>> {
        Ok(Box::new(InMemorySegment { data: Vec::new() }))
    }

    fn append(&mut self, data: &Vec<u8>) -> LogResult<u64> {
        self.data.append(&mut data.clone());
        Ok(self.data.len() as u64)
    }

    fn lookup(&mut self, size: u64, offset: u64) -> LogResult<Vec<u8>> {
        if size + offset > self.data.len() as u64 {
            Err(LogError::Eof)
        } else {
            Ok(self.data[offset as usize..(size + offset) as usize].to_vec())
        }
    }

    fn modified(&self) -> LogResult<SystemTime> {
        Ok(SystemTime::now())
    }

    fn remove(&self) -> LogResult<()> {
        Ok(())
    }

    fn read_dir(_path: &Path) -> LogResult<ReadDir> {  // TODO(AD) ties to fs
        Err(LogError::Eof)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn append_and_lookup() {
        let mut segment = InMemorySegment::open(&Path::new("")).unwrap();

        let written = vec![1, 2, 3];
        segment.append(&written).unwrap();

        let read = segment.lookup(3, 0).unwrap();
        assert_eq!(written, read);
    }

    #[test]
    fn lookup_eof() {
        let mut segment = InMemorySegment::open(&Path::new("")).unwrap();
        if let Err(LogError::Eof) = segment.lookup(4, 0) {
        } else {
            panic!("expected segment expired");
        }
    }
}
