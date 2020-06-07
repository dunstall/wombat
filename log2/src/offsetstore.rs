use byteorder::{BigEndian, ReadBytesExt, WriteBytesExt};
use std::collections::BTreeMap;
use std::io::Cursor;

use crate::result::LogResult;
use crate::segment::Segment;

// Handles lookup of the segment that owns a given offset. All entries are
// persisted to the segment so they can be loaded on startup.
pub struct OffsetStore<S> {
    offsets: BTreeMap<u64, u64>,
    segment: Box<S>,
}

// TODO Will be an issue when multiple logs refer to the same data.
impl<S> OffsetStore<S>
where
    S: Segment,
{
    pub async fn new(segment: Box<S>) -> LogResult<OffsetStore<S>> {
        let mut offsets = OffsetStore {
            offsets: BTreeMap::new(),
            segment,
        };
        offsets.load_offsets().await?;
        Ok(offsets)
    }

    // Returns the segment ID and starting offset of this segment.
    pub fn get(&self, offset: u64) -> Option<(u64, u64)> {
        for (first_offset, id) in self.offsets.iter().rev() {
            if *first_offset <= offset {
                return Some((*id, *first_offset as u64));
            }
        }
        None
    }

    // Inserts the given ID name and starting offset. This will be
    // persisted to the file before updating in memory.
    pub async fn insert(&mut self, offset: u64, id: u64) -> LogResult<()> {
        self.write_u64(offset as u64).await?;
        self.write_u64(id).await?;
        self.offsets.insert(offset, id);
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
    async fn load_offsets(&mut self) -> LogResult<()> {
        let mut offset = 0;
        loop {
            if let Err(_) = self.load_offset(offset).await {
                return Ok(());
            }
            offset += 16;
        }
    }

    async fn load_offset(&mut self, from: u64) -> LogResult<()> {
        let offset = self.read_u64(from).await?;
        let id = self.read_u64(from + 8).await?;
        self.offsets.insert(offset, id);
        Ok(())
    }

    async fn read_u64(&mut self, offset: u64) -> LogResult<u64> {
        let mut rdr = Cursor::new(self.segment.lookup(offset, 8).await?);
        Ok(rdr.read_u64::<BigEndian>()?)
    }

    async fn write_u64(&mut self, n: u64) -> LogResult<()> {
        let mut size = vec![];
        size.write_u64::<BigEndian>(n)?;
        self.segment.append(&size).await?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use crate::inmemorysegment::InMemorySegment;
    use std::path::Path;

    #[tokio::test]
    async fn lookup_empty() {
        let seg = InMemorySegment::open(0x43f3, Path::new("foo/bar"))
            .await
            .unwrap();
        let offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();

        assert_eq!(offsets.get(0), None);
        assert_eq!(offsets.get(100), None);
    }

    #[tokio::test]
    async fn lookup_zero_offset() {
        let seg = InMemorySegment::open(0x1666, Path::new("foo/bar"))
            .await
            .unwrap();
        let mut offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();

        let id = 0;
        offsets.insert(0, id).await.unwrap();

        assert_eq!(offsets.get(0), Some((id, 0)));
        assert_eq!(offsets.get(0xff), Some((id, 0)));
    }

    #[tokio::test]
    async fn lookup_non_zero_offset() {
        let seg = InMemorySegment::open(0xf01f, Path::new("foo/bar"))
            .await
            .unwrap();
        let mut offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();

        let id = 0;

        offsets.insert(0xaa, id).await.unwrap();

        assert_eq!(offsets.get(0), None);
        assert_eq!(offsets.get(0xaa), Some((0, 0xaa)));
        assert_eq!(offsets.get(0xff), Some((0, 0xaa)));
    }

    #[tokio::test]
    async fn lookup_multi_offset() {
        let seg = InMemorySegment::open(0xa9ea, Path::new("foo/bar"))
            .await
            .unwrap();
        let mut offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();

        let id1 = 0;
        let id2 = 1;
        offsets.insert(0xa0, id1).await.unwrap();
        offsets.insert(0xb0, id2).await.unwrap();

        assert_eq!(offsets.get(0x9f), None);
        assert_eq!(offsets.get(0xa0), Some((id1, 0xa0)));
        assert_eq!(offsets.get(0xaf), Some((id1, 0xa0)));
        assert_eq!(offsets.get(0xb0), Some((id2, 0xb0)));
        assert_eq!(offsets.get(0xff), Some((id2, 0xb0)));
    }

    #[tokio::test]
    async fn lookup_reverse_offset() {
        let seg = InMemorySegment::open(0xfeaa, Path::new("foo/bar"))
            .await
            .unwrap();
        let mut offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();

        let id1 = 0;
        let id2 = 1;
        let id3 = 2;
        offsets.insert(0x20, id3).await.unwrap();
        offsets.insert(0x10, id2).await.unwrap();
        offsets.insert(0x00, id1).await.unwrap();

        assert_eq!(offsets.get(0x00), Some((id1, 0x00)));
        assert_eq!(offsets.get(0x0f), Some((id1, 0x00)));
        assert_eq!(offsets.get(0x10), Some((id2, 0x10)));
        assert_eq!(offsets.get(0x1f), Some((id2, 0x10)));
        assert_eq!(offsets.get(0x20), Some((id3, 0x20)));
        assert_eq!(offsets.get(0x2f), Some((id3, 0x20)));
    }

    #[tokio::test]
    async fn max_offset() {
        let seg = InMemorySegment::open(0x7e0e, Path::new("foo/bar"))
            .await
            .unwrap();
        let mut offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();

        assert_eq!(offsets.max_offset(), 0);

        offsets.insert(0x00, 0).await.unwrap();
        assert_eq!(offsets.max_offset(), 0);
        offsets.insert(0x10, 1).await.unwrap();
        assert_eq!(offsets.max_offset(), 0x10);
        offsets.insert(0x20, 2).await.unwrap();
        assert_eq!(offsets.max_offset(), 0x20);
    }

    #[tokio::test]
    async fn load_persisted() {
        let id1 = 0;
        let id2 = 1;

        {
            let seg = InMemorySegment::open(0x2065, Path::new("foo/bar"))
                .await
                .unwrap();
            let mut offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();
            offsets.insert(0xa0, id1).await.unwrap();
        }

        {
            let seg = InMemorySegment::open(0x2065, Path::new("foo/bar"))
                .await
                .unwrap();
            let mut offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();
            offsets.insert(0xb0, id2).await.unwrap();
        }

        {
            let seg = InMemorySegment::open(0x2065, Path::new("foo/bar"))
                .await
                .unwrap();
            let offsets = OffsetStore::<InMemorySegment>::new(seg).await.unwrap();
            assert_eq!(offsets.get(0x9f), None);
            assert_eq!(offsets.get(0xa0), Some((id1, 0xa0)));
            assert_eq!(offsets.get(0xaf), Some((id1, 0xa0)));
            assert_eq!(offsets.get(0xb0), Some((id2, 0xb0)));
            assert_eq!(offsets.get(0xff), Some((id2, 0xb0)));
        }
    }
}
