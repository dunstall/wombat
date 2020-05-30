use byteorder::{BigEndian, ReadBytesExt, WriteBytesExt};
use std::collections::BTreeMap;
use std::io::{Read, Write};
use std::string::String;
use std::vec::Vec;

use crate::result::LogResult;

// Handles lookup of the segment that owns a given offset. All entries are
// written to a file so they can be loaded on startup.
pub struct OffsetStore<F> {
    offsets: BTreeMap<usize, String>,
    file: F,
}

impl<F> OffsetStore<F>
where
    F: Read + Write,
{
    pub fn new(file: F) -> LogResult<OffsetStore<F>> {
        let mut offsets = OffsetStore {
            offsets: BTreeMap::new(),
            file: file,
        };
        offsets.load_offsets()?;
        Ok(offsets)
    }

    // Returns the name of the segment containing this offset.
    pub fn get(&self, offset: usize) -> Option<&String> {
        for (first_offset, segment) in self.offsets.iter().rev() {
            if *first_offset <= offset {
                return Some(segment);
            }
        }
        None
    }

    // Inserts the given segment name and starting offset. This will be
    // persisted to the file before updating in memory.
    pub fn insert(&mut self, offset: usize, segment: String) -> LogResult<()> {
        self.file.write_u64::<BigEndian>(offset as u64)?;
        self.file.write_u64::<BigEndian>(segment.len() as u64)?;
        self.file.write_all(&segment.as_bytes().to_vec())?;
        self.offsets.insert(offset, segment);
        Ok(())
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
        let offset = self.read_offset()?;
        let segment = self.read_segment()?;
        self.offsets.insert(offset, segment);
        Ok(())
    }

    fn read_offset(&mut self) -> LogResult<usize> {
        Ok(self.file.read_u64::<BigEndian>()? as usize)
    }

    fn read_segment(&mut self) -> LogResult<String> {
        let size = self.file.read_u64::<BigEndian>()?;
        let mut buffer = Vec::new();
        buffer.resize(size as usize, 0);
        self.file.read_exact(&mut buffer[..])?;
        Ok(String::from_utf8(buffer)?)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::fs::File;
    use std::io::Cursor;
    use tempdir::TempDir;

    #[test]
    fn lookup_empty() {
        let offsets = OffsetStore::new(Cursor::new(vec![])).unwrap();
        assert_eq!(offsets.get(0), None);
        assert_eq!(offsets.get(100), None);
    }

    #[test]
    fn lookup_zero_offset() {
        let segment = "mysegment".to_string();
        let mut offsets = OffsetStore::new(Cursor::new(vec![])).unwrap();
        offsets.insert(0, segment.clone()).unwrap();

        assert_eq!(offsets.get(0), Some(&segment));
        assert_eq!(offsets.get(0xaa), Some(&segment));
        assert_eq!(offsets.get(0xff), Some(&segment));
    }

    #[test]
    fn lookup_big_offset() {
        let segment = "mysegment".to_string();
        let mut offsets = OffsetStore::new(Cursor::new(vec![])).unwrap();
        offsets.insert(0xaa, segment.clone()).unwrap();

        assert_eq!(offsets.get(0), None);
        assert_eq!(offsets.get(0xaa), Some(&segment));
        assert_eq!(offsets.get(0xff), Some(&segment));
    }

    #[test]
    fn lookup_multi_offset() {
        let segment1 = "mysegment1".to_string();
        let segment2 = "mysegment2".to_string();
        let mut offsets = OffsetStore::new(Cursor::new(vec![])).unwrap();
        offsets.insert(0xa0, segment1.clone()).unwrap();
        offsets.insert(0xb0, segment2.clone()).unwrap();

        assert_eq!(offsets.get(0x9f), None);
        assert_eq!(offsets.get(0xa0), Some(&segment1));
        assert_eq!(offsets.get(0xaf), Some(&segment1));
        assert_eq!(offsets.get(0xb0), Some(&segment2));
        assert_eq!(offsets.get(0xff), Some(&segment2));
    }

    #[test]
    fn lookup_reverse_offset() {
        let segment1 = "mysegment1".to_string();
        let segment2 = "mysegment2".to_string();
        let segment3 = "mysegment3".to_string();
        let mut offsets = OffsetStore::new(Cursor::new(vec![])).unwrap();
        offsets.insert(0x20, segment3.clone()).unwrap();
        offsets.insert(0x10, segment2.clone()).unwrap();
        offsets.insert(0x00, segment1.clone()).unwrap();

        assert_eq!(offsets.get(0x00), Some(&segment1));
        assert_eq!(offsets.get(0x0f), Some(&segment1));
        assert_eq!(offsets.get(0x10), Some(&segment2));
        assert_eq!(offsets.get(0x1f), Some(&segment2));
        assert_eq!(offsets.get(0x20), Some(&segment3));
        assert_eq!(offsets.get(0x2f), Some(&segment3));
    }

    #[test]
    fn load_from_file() {
        let tmp_dir = TempDir::new("log-unit-tests").unwrap();
        let file_path = tmp_dir.path().join("offsets");

        let segment1 = "mysegment1".to_string();
        let segment2 = "mysegment2".to_string();

        {
            let buf = File::create(file_path.clone()).unwrap();
            let mut offsets = OffsetStore::new(buf).unwrap();
            offsets.insert(0xa0, segment1.clone()).unwrap();
            offsets.insert(0xb0, segment2.clone()).unwrap();
        }

        {
            let buf = File::open(file_path).unwrap();
            let offsets = OffsetStore::new(buf).unwrap();

            assert_eq!(offsets.get(0x9f), None);
            assert_eq!(offsets.get(0xa0), Some(&segment1));
            assert_eq!(offsets.get(0xaf), Some(&segment1));
            assert_eq!(offsets.get(0xb0), Some(&segment2));
            assert_eq!(offsets.get(0xff), Some(&segment2));
        }
    }
}
