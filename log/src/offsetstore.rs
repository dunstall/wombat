use byteorder::{BigEndian, ReadBytesExt, WriteBytesExt};
use std::collections::BTreeMap;
use std::io::Cursor;

use crate::result::LogResult;
use crate::segment::Segment;

// Handles lookup of the segment that owns a given offset. All entries are
// persisted to the segment so they can be loaded on startup.
pub struct OffsetStore {
    offsets: BTreeMap<u64, u64>,
    segment: Box<dyn Segment>,
}

impl OffsetStore {
    pub fn new(segment: Box<dyn Segment>) -> LogResult<OffsetStore> {
        let mut offsets = OffsetStore {
            offsets: BTreeMap::new(),
            segment,
        };
        offsets.load_offsets()?;
        Ok(offsets)
    }

    // Returns the segment number and starting offset of this segment.
    pub fn get(&self, offset: u64) -> Option<(u64, u64)> {
        for (first_offset, id) in self.offsets.iter().rev() {
            if *first_offset <= offset {
                return Some((*id, *first_offset as u64));
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

    pub fn max_offset(&self) -> u64 {
        if let Some(max_offset) = self.offsets.iter().next_back() {
            *max_offset.0
        } else {
            0
        }
    }

    // TODO(AD) Not handling errors if file format.
    fn load_offsets(&mut self) -> LogResult<()> {
        let mut offset = 0;
        loop {
            if let Err(_) = self.load_offset(offset) {
                return Ok(());
            }
            offset += 16;
        }
    }

    fn load_offset(&mut self, from: u64) -> LogResult<()> {
        let offset = self.read_u64(from)?;
        let segment = self.read_u64(from + 8)?;
        self.offsets.insert(offset, segment);
        Ok(())
    }

    fn read_u64(&mut self, offset: u64) -> LogResult<u64> {
        let mut rdr = Cursor::new(self.segment.lookup(8, offset)?);
        Ok(rdr.read_u64::<BigEndian>()?)
    }

    fn write_u64(&mut self, n: u64) -> LogResult<()> {
        let mut size = vec![];
        size.write_u64::<BigEndian>(n)?;
        self.segment.append(&size)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use crate::inmemorysegmentmanager::InMemorySegmentManager;
    use crate::segmentmanager::SegmentManager;

    #[test]
    fn lookup_empty() {
        let mut manager = Box::new(InMemorySegmentManager::new().unwrap());
        let segment = manager.open_name("segment.offsets".to_string()).unwrap();
        let offsets = OffsetStore::new(segment).unwrap();

        assert_eq!(offsets.get(0), None);
        assert_eq!(offsets.get(100), None);
    }

    #[test]
    fn lookup_zero_offset() {
        let mut manager = Box::new(InMemorySegmentManager::new().unwrap());
        let segment = manager.open_name("segment.offsets".to_string()).unwrap();
        let mut offsets = OffsetStore::new(segment).unwrap();

        let id = 0;
        offsets.insert(0, id).unwrap();

        assert_eq!(offsets.get(0), Some((id, 0)));
        assert_eq!(offsets.get(0xff), Some((id, 0)));
    }

    #[test]
    fn lookup_non_zero_offset() {
        let mut manager = Box::new(InMemorySegmentManager::new().unwrap());
        let segment = manager.open_name("segment.offsets".to_string()).unwrap();
        let mut offsets = OffsetStore::new(segment).unwrap();

        let id = 0;

        offsets.insert(0xaa, id).unwrap();

        assert_eq!(offsets.get(0), None);
        assert_eq!(offsets.get(0xaa), Some((0, 0xaa)));
        assert_eq!(offsets.get(0xff), Some((0, 0xaa)));
    }

    #[test]
    fn lookup_multi_offset() {
        let mut manager = Box::new(InMemorySegmentManager::new().unwrap());
        let segment = manager.open_name("segment.offsets".to_string()).unwrap();
        let mut offsets = OffsetStore::new(segment).unwrap();

        let id1 = 0;
        let id2 = 1;
        offsets.insert(0xa0, id1).unwrap();
        offsets.insert(0xb0, id2).unwrap();

        assert_eq!(offsets.get(0x9f), None);
        assert_eq!(offsets.get(0xa0), Some((id1, 0xa0)));
        assert_eq!(offsets.get(0xaf), Some((id1, 0xa0)));
        assert_eq!(offsets.get(0xb0), Some((id2, 0xb0)));
        assert_eq!(offsets.get(0xff), Some((id2, 0xb0)));
    }

    #[test]
    fn lookup_reverse_offset() {
        let mut manager = Box::new(InMemorySegmentManager::new().unwrap());
        let segment = manager.open_name("segment.offsets".to_string()).unwrap();
        let mut offsets = OffsetStore::new(segment).unwrap();

        let id1 = 0;
        let id2 = 1;
        let id3 = 2;
        offsets.insert(0x20, id3).unwrap();
        offsets.insert(0x10, id2).unwrap();
        offsets.insert(0x00, id1).unwrap();

        assert_eq!(offsets.get(0x00), Some((id1, 0x00)));
        assert_eq!(offsets.get(0x0f), Some((id1, 0x00)));
        assert_eq!(offsets.get(0x10), Some((id2, 0x10)));
        assert_eq!(offsets.get(0x1f), Some((id2, 0x10)));
        assert_eq!(offsets.get(0x20), Some((id3, 0x20)));
        assert_eq!(offsets.get(0x2f), Some((id3, 0x20)));
    }

    #[test]
    fn max_offset() {
        let mut manager = Box::new(InMemorySegmentManager::new().unwrap());
        let segment = manager.open_name("segment.offsets".to_string()).unwrap();
        let mut offsets = OffsetStore::new(segment).unwrap();

        assert_eq!(offsets.max_offset(), 0);

        offsets.insert(0x00, 0).unwrap();
        assert_eq!(offsets.max_offset(), 0);
        offsets.insert(0x10, 1).unwrap();
        assert_eq!(offsets.max_offset(), 0x10);
        offsets.insert(0x20, 2).unwrap();
        assert_eq!(offsets.max_offset(), 0x20);
    }

    #[test]
    fn load_persisted() {
        let mut manager = Box::new(InMemorySegmentManager::new().unwrap());

        let id1 = 0;
        let id2 = 1;

        {
            let segment = manager.open_name("segment.offsets".to_string()).unwrap();
            let mut offsets = OffsetStore::new(segment).unwrap();
            offsets.insert(0xa0, id1).unwrap();
        }

        {
            let segment = manager.open_name("segment.offsets".to_string()).unwrap();
            let mut offsets = OffsetStore::new(segment).unwrap();
            offsets.insert(0xb0, id2).unwrap();
        }

        {
            let segment = manager.open_name("segment.offsets".to_string()).unwrap();
            let offsets = OffsetStore::new(segment).unwrap();
            assert_eq!(offsets.get(0x9f), None);
            assert_eq!(offsets.get(0xa0), Some((id1, 0xa0)));
            assert_eq!(offsets.get(0xaf), Some((id1, 0xa0)));
            assert_eq!(offsets.get(0xb0), Some((id2, 0xb0)));
            assert_eq!(offsets.get(0xff), Some((id2, 0xb0)));
        }
    }
}
