use std::fs::File;
use std::io;
use std::io::{Read, Seek, SeekFrom, Write};
use std::time::SystemTime;
use std::vec::Vec;

use crate::result::{LogError, LogResult};
use crate::segment::Segment;

pub struct SystemSegment {
    file: File,
}

impl SystemSegment {
    pub fn new(file: File) -> SystemSegment {
        SystemSegment { file }
    }
}

// TODO(AD) Handle flush/sync
impl Segment for SystemSegment {
    fn append(&mut self, data: &Vec<u8>) -> LogResult<u64> {
        self.file.write_all(data)?;
        Ok(self.file.seek(SeekFrom::Current(0))?)
    }

    fn lookup(&mut self, size: u64, offset: u64) -> LogResult<Vec<u8>> {
        self.file.seek(SeekFrom::Start(offset as u64))?;
        let mut buf = Vec::new();
        buf.resize(size as usize, 0);

        if let Err(err) = self.file.read_exact(&mut buf[..]) {
            if err.kind() == io::ErrorKind::UnexpectedEof {
                Err(LogError::Eof)
            } else {
                Err(LogError::IoError(err))
            }
        } else {
            Ok(buf)
        }
    }

    fn modified(&self) -> LogResult<SystemTime> {
        Ok(self.file.metadata()?.modified()?)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use tempdir::TempDir;

    use crate::segmentmanager::SegmentManager;
    use crate::systemsegmentmanager::SystemSegmentManager;

    #[test]
    fn lookup_not_empty() {
        let dir = tmp_dir();
        let mut manager = SystemSegmentManager::new(&dir.path()).unwrap();
        let mut segment = manager.open(0xaabb).unwrap();

        segment.append(&vec![1, 2, 3]).unwrap();
        segment.append(&vec![4, 5, 6]).unwrap();

        assert_eq!(vec![1, 2, 3], segment.lookup(3, 0).unwrap());
        assert_eq!(vec![5, 6], segment.lookup(2, 4).unwrap());
        assert_eq!(vec![4], segment.lookup(1, 3).unwrap());
    }

    #[test]
    fn lookup_empty() {
        let dir = tmp_dir();
        let mut manager = SystemSegmentManager::new(&dir.path()).unwrap();
        let mut segment = manager.open(0xaabb).unwrap();

        if let Err(LogError::Eof) = segment.lookup(4, 0) {
        } else {
            panic!("expected segment expired");
        }
    }

    fn tmp_dir() -> TempDir {
        TempDir::new("dingolog").unwrap()
    }
}
