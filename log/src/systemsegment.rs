use std::fs;
use std::fs::{File, OpenOptions};
use std::io;
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::Path;
use std::time::SystemTime;
use std::vec::Vec;

use crate::result::{LogError, LogResult};
use crate::segment;
use crate::segment::Segment;

pub struct SystemSegment {
    file: File,
    path: String,
}

// TODO(AD) Handle flush/sync
impl Segment for SystemSegment {
    fn open(path: &Path) -> LogResult<Box<SystemSegment>> {
        fs::create_dir_all(path.parent().unwrap())?;
        let file = OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .append(true)
            .open(path)?;
        Ok(Box::new(SystemSegment {
            file,
            path: path.to_str().unwrap().to_string(),
        }))
    }

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

    // TODO(AD) Test
    fn modified(&self) -> LogResult<SystemTime> {
        Ok(self.file.metadata()?.modified()?)
    }

    // TODO(AD) Test
    fn remove(&self) -> LogResult<()> {
        Ok(fs::remove_file(&self.path)?)
    }

    fn read_dir(dir: &Path) -> LogResult<Vec<u64>> {
        let mut ids = Vec::new();
        for entry in fs::read_dir(dir)? {
            let entry = entry?;
            let name = entry.file_name().into_string()?;
            if let Some(id) = segment::name_to_id(&name) {
                ids.push(id);
            }
        }
        Ok(ids)
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
        let mut segment = SystemSegment::open(&tmp.path().join("segment.1")).unwrap();

        let written = vec![1, 2, 3];
        assert_eq!(segment.append(&written).unwrap(), 3);

        let read = segment.lookup(3, 0).unwrap();
        assert_eq!(written, read);
    }

    #[test]
    fn append_multi() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut segment = SystemSegment::open(&tmp.path().join("segment.1")).unwrap();

        for offset in (0..1000).step_by(100) {
            let mut rng = rand::thread_rng();
            let written: Vec<u8> = (0..100).map(|_| rng.gen_range(0, 0xff)).collect();

            assert_eq!(segment.append(&written).unwrap(), offset + 100);

            let read = segment.lookup(100, offset).unwrap();
            assert_eq!(written, read);
        }
    }

    #[test]
    fn append_multi_unordered_read() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut segment = SystemSegment::open(&tmp.path().join("segment.1")).unwrap();

        assert_eq!(segment.append(&vec![1, 2, 3]).unwrap(), 3);
        assert_eq!(segment.append(&vec![4, 5, 6]).unwrap(), 6);

        assert_eq!(vec![1, 2, 3], segment.lookup(3, 0).unwrap());
        assert_eq!(vec![4, 5, 6], segment.lookup(3, 3).unwrap());
        assert_eq!(vec![1, 2, 3], segment.lookup(3, 0).unwrap());

        assert_eq!(segment.append(&vec![7, 8, 9]).unwrap(), 9);

        assert_eq!(vec![1, 2, 3], segment.lookup(3, 0).unwrap());
        assert_eq!(vec![4, 5, 6], segment.lookup(3, 3).unwrap());
        assert_eq!(vec![7, 8, 9], segment.lookup(3, 6).unwrap());
    }

    #[test]
    fn lookup_eof() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut segment = SystemSegment::open(&tmp.path().join("segment.1")).unwrap();

        if let Err(LogError::Eof) = segment.lookup(4, 0) {
        } else {
            panic!("expected EOF");
        }
    }
}
