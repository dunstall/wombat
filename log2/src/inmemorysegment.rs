use async_trait::async_trait;
use lazy_static::lazy_static;
use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::sync::Mutex;

use crate::result::{LogError, LogResult};
use crate::segment;
use crate::Segment;

lazy_static! {
    // State stores the shared state of in-memory segments to act like a
    // persistent filesystem.
    static ref STATE: Mutex<HashMap<PathBuf, Vec<u8>>> = Mutex::new(HashMap::new());
}

/// Implementation of Segment using an in-memory buffer.
///
/// This must only be used for testing. To act as a file system the segments
/// state is stored in a (syncronized) global buffer indexed by `id` and `dir`
/// so these must be unique to avoid collisions (which is annoying but a
/// reasonable trade-off).
///
/// Not thread safe.
pub struct InMemorySegment {
    path: PathBuf,
}

#[async_trait]
impl Segment for InMemorySegment {
    async fn open(id: u64, dir: &Path) -> LogResult<Box<Self>> {
        Ok(Box::new(InMemorySegment {
            path: dir.join(segment::id_to_name(id)),
        }))
    }

    async fn append(&mut self, data: &Vec<u8>) -> LogResult<()> {
        let mut state = STATE.lock().unwrap();
        if let Some(buf) = state.get_mut(&self.path) {
            buf.append(&mut data.clone());
        } else {
            state.insert(self.path.clone(), data.clone());
        }
        Ok(())
    }

    async fn lookup(&mut self, offset: u64, size: u64) -> LogResult<Vec<u8>> {
        if let Some(buf) = STATE.lock().unwrap().get(&self.path) {
            if offset + size <= buf.len() as u64 {
                Ok(buf[offset as usize..(offset + size) as usize].to_vec())
            } else {
                Err(LogError::Eof)
            }
        } else {
            Ok(vec![])
        }
    }

    async fn size(&mut self) -> LogResult<u64> {
        if let Some(buf) = STATE.lock().unwrap().get(&self.path) {
            Ok(buf.len() as u64)
        } else {
            Ok(0)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn open_empty() {
        let mut seg = InMemorySegment::open(0x8312, Path::new("foo/bar"))
            .await
            .unwrap();
        assert_eq!(0, seg.size().await.unwrap());
    }

    #[tokio::test]
    async fn open_load_existing() {
        let mut seg = InMemorySegment::open(0xfa89, Path::new("foo/bar"))
            .await
            .unwrap();
        seg.append(&vec![1, 2, 3]).await.unwrap();

        // Re-open the non-empty segment.
        let mut seg = InMemorySegment::open(0xfa89, Path::new("foo/bar"))
            .await
            .unwrap();
        assert_eq!(3, seg.size().await.unwrap());
        assert_eq!(vec![1, 2, 3], seg.lookup(0, 3).await.unwrap());
    }

    // Test multiple segments to check no collisions.
    #[tokio::test]
    async fn multi_segment_lookup_offset() {
        let mut seg1 = InMemorySegment::open(0xd6a2, Path::new("foo/bar"))
            .await
            .unwrap();
        seg1.append(&vec![1, 2, 3]).await.unwrap();
        assert_eq!(3, seg1.size().await.unwrap());
        assert_eq!(vec![1, 2, 3], seg1.lookup(0, 3).await.unwrap());

        let mut seg2 = InMemorySegment::open(0xb2aa, Path::new("foo/bar"))
            .await
            .unwrap();
        seg2.append(&vec![6, 7, 8, 9]).await.unwrap();
        assert_eq!(4, seg2.size().await.unwrap());
        assert_eq!(vec![6, 7, 8, 9], seg2.lookup(0, 4).await.unwrap());
    }

    // Test multiple segments refering to the same data.
    #[tokio::test]
    async fn multi_segments_shared() {
        let mut seg1 = InMemorySegment::open(0xf2aa, Path::new("foo/bar"))
            .await
            .unwrap();
        seg1.append(&vec![1, 2, 3]).await.unwrap();

        let mut seg2 = InMemorySegment::open(0xf2aa, Path::new("foo/bar"))
            .await
            .unwrap();
        assert_eq!(3, seg2.size().await.unwrap());
        assert_eq!(vec![1, 2, 3], seg2.lookup(0, 3).await.unwrap());
        seg2.append(&vec![4, 5, 6]).await.unwrap();

        assert_eq!(6, seg1.size().await.unwrap());
        assert_eq!(vec![4, 5, 6], seg1.lookup(3, 3).await.unwrap());
    }
}
