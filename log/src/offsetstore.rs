use byteorder::{BigEndian, ReadBytesExt, WriteBytesExt};
use std::collections::BTreeMap;
use std::fs::{File, OpenOptions};
use std::path::Path;

use crate::result::LogResult;

// Handles lookup of the segment that owns a given offset. All entries are
// written to a file so they can be loaded on startup.
#[derive(Debug)]
pub struct OffsetStore {
    // TODO(AD) Replace all usize with u64
    offsets: BTreeMap<u64, u64>,
    file: File,
}

impl OffsetStore {
    pub fn new(save_path: &Path) -> LogResult<OffsetStore> {
        let file = OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .append(true)
            .open(save_path)?;
        let mut offsets = OffsetStore {
            offsets: BTreeMap::new(),
            file: file,
        };
        offsets.load_offsets()?;
        Ok(offsets)
    }

    // Returns the segment number and starting offset of this segment.
    pub fn get(&self, offset: u64) -> Option<(u64, u64)> {
        for (first_offset, segment) in self.offsets.iter().rev() {
            if *first_offset <= offset {
                return Some((*segment, *first_offset as u64));
            }
        }
        None
    }

    // Inserts the given segment name and starting offset. This will be
    // persisted to the file before updating in memory.
    pub fn insert(&mut self, offset: u64, segment: u64) -> LogResult<()> {
        self.write_u64(offset as u64)?;
        self.write_u64(segment)?;
        self.offsets.insert(offset, segment);
        Ok(())
    }

    pub fn max_offset(&mut self) -> u64 {
        if let Some(max_offset) = self.offsets.iter().next_back() {
            *max_offset.0
        } else {
            0
        }
    }

    // TODO(AD) Not handling errors if file format.
    fn load_offsets(&mut self) -> LogResult<()> {
        loop {
            if let Err(_) = self.load_offset() {
                return Ok(());
            }
        }
    }

    fn load_offset(&mut self) -> LogResult<()> {
        let offset = self.read_u64()?;
        let segment = self.read_u64()?;
        self.offsets.insert(offset, segment);
        Ok(())
    }

    fn read_u64(&mut self) -> LogResult<u64> {
        Ok(self.file.read_u64::<BigEndian>()?)
    }

    fn write_u64(&mut self, n: u64) -> LogResult<()> {
        self.file.write_u64::<BigEndian>(n)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use tempdir::TempDir;

    #[test]
    fn lookup_empty() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let path = tmp.path().join("offsets");

        let offsets = OffsetStore::new(&path).unwrap();
        assert_eq!(offsets.get(0), None);
        assert_eq!(offsets.get(100), None);
    }

    #[test]
    fn lookup_zero_offset() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let path = tmp.path().join("offsets");

        let segment = 0;
        let mut offsets = OffsetStore::new(&path).unwrap();
        offsets.insert(0, segment).unwrap();

        assert_eq!(offsets.get(0), Some((segment, 0)));
        assert_eq!(offsets.get(0xaa), Some((segment, 0)));
        assert_eq!(offsets.get(0xff), Some((segment, 0)));
    }

    #[test]
    fn lookup_big_offset() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let path = tmp.path().join("offsets");

        let segment = 0;
        let mut offsets = OffsetStore::new(&path).unwrap();
        offsets.insert(0xaa, segment).unwrap();

        assert_eq!(offsets.get(0), None);
        assert_eq!(offsets.get(0xaa), Some((segment, 0xaa)));
        assert_eq!(offsets.get(0xff), Some((segment, 0xaa)));
    }

    #[test]
    fn lookup_multi_offset() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let path = tmp.path().join("offsets");

        let segment1 = 0;
        let segment2 = 1;
        let mut offsets = OffsetStore::new(&path).unwrap();
        offsets.insert(0xa0, segment1).unwrap();
        offsets.insert(0xb0, segment2).unwrap();

        assert_eq!(offsets.get(0x9f), None);
        assert_eq!(offsets.get(0xa0), Some((segment1, 0xa0)));
        assert_eq!(offsets.get(0xaf), Some((segment1, 0xa0)));
        assert_eq!(offsets.get(0xb0), Some((segment2, 0xb0)));
        assert_eq!(offsets.get(0xff), Some((segment2, 0xb0)));
    }

    #[test]
    fn lookup_reverse_offset() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let path = tmp.path().join("offsets");

        let segment1 = 0;
        let segment2 = 1;
        let segment3 = 2;
        let mut offsets = OffsetStore::new(&path).unwrap();
        offsets.insert(0x20, segment3).unwrap();
        offsets.insert(0x10, segment2).unwrap();
        offsets.insert(0x00, segment1).unwrap();

        assert_eq!(offsets.get(0x00), Some((segment1, 0x00)));
        assert_eq!(offsets.get(0x0f), Some((segment1, 0x00)));
        assert_eq!(offsets.get(0x10), Some((segment2, 0x10)));
        assert_eq!(offsets.get(0x1f), Some((segment2, 0x10)));
        assert_eq!(offsets.get(0x20), Some((segment3, 0x20)));
        assert_eq!(offsets.get(0x2f), Some((segment3, 0x20)));
    }

    #[test]
    fn max_offset() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let path = tmp.path().join("offsets");
        let mut offsets = OffsetStore::new(&path).unwrap();

        assert_eq!(offsets.max_offset(), 0);

        offsets.insert(0x00, 0).unwrap();
        assert_eq!(offsets.max_offset(), 0);
        offsets.insert(0x10, 1).unwrap();
        assert_eq!(offsets.max_offset(), 0x10);
        offsets.insert(0x20, 2).unwrap();
        assert_eq!(offsets.max_offset(), 0x20);
    }

    #[test]
    fn load_from_file() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let path = tmp.path().join("offsets");

        let segment1 = 0;
        let segment2 = 1;

        {
            let mut offsets = OffsetStore::new(&path).unwrap();
            offsets.insert(0xa0, segment1).unwrap();
            offsets.insert(0xb0, segment2).unwrap();
        }

        {
            let offsets = OffsetStore::new(&path).unwrap();

            assert_eq!(offsets.get(0x9f), None);
            assert_eq!(offsets.get(0xa0), Some((segment1, 0xa0)));
            assert_eq!(offsets.get(0xaf), Some((segment1, 0xa0)));
            assert_eq!(offsets.get(0xb0), Some((segment2, 0xb0)));
            assert_eq!(offsets.get(0xff), Some((segment2, 0xb0)));
        }
    }

    #[test]
    fn multi_load_from_file() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let path = tmp.path().join("offsets");

        let segment1 = 0;
        let segment2 = 1;

        {
            let mut offsets = OffsetStore::new(&path).unwrap();
            offsets.insert(0xa0, segment1).unwrap();
        }

        {
            let mut offsets = OffsetStore::new(&path).unwrap();
            offsets.insert(0xb0, segment2).unwrap();
        }

        {
            let offsets = OffsetStore::new(&path).unwrap();

            assert_eq!(offsets.get(0x9f), None);
            assert_eq!(offsets.get(0xa0), Some((segment1, 0xa0)));
            assert_eq!(offsets.get(0xaf), Some((segment1, 0xa0)));
            assert_eq!(offsets.get(0xb0), Some((segment2, 0xb0)));
            assert_eq!(offsets.get(0xff), Some((segment2, 0xb0)));
        }
    }
}
