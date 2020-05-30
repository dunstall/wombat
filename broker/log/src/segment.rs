use std::fs::{File, OpenOptions};
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::Path;
use std::vec::Vec;

use crate::result::LogResult;

pub struct Segment {
    file: File,
}

// TODO Handle flush/sync
impl Segment {
    pub fn new(path: &Path) -> LogResult<Segment> {
        let file = OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .append(true)
            .open(path)?;
        Ok(Segment { file })
    }

    pub fn append(&mut self, data: &Vec<u8>) -> LogResult<()> {
        self.file.write_all(data)?;
        Ok(())
    }

    pub fn lookup(&mut self, size: usize, offset: usize) -> LogResult<Vec<u8>> {
        self.file.seek(SeekFrom::Start(offset as u64))?;
        let mut buf = Vec::new();
        buf.resize(size, 0);
        self.file.read_exact(&mut buf[..])?;
        Ok(buf)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use rand::Rng;
    use tempdir::TempDir;

    #[test]
    fn append_single() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut segment = Segment::new(&tmp.path().join("segment.1")).unwrap();

        let written = vec![1, 2, 3];
        segment.append(&written).unwrap();

        let read = segment.lookup(3, 0).unwrap();
        assert_eq!(written, read);
    }

    #[test]
    fn append_multi() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut segment = Segment::new(&tmp.path().join("segment.1")).unwrap();

        for offset in (0..1000).step_by(100) {
            let mut rng = rand::thread_rng();
            let written: Vec<u8> = (0..100).map(|_| {
                rng.gen_range(0, 0xff)
            }).collect();

            segment.append(&written).unwrap();

            let read = segment.lookup(100, offset).unwrap();
            assert_eq!(written, read);
        }
    }
}
