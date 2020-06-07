use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::vec::Vec;

use crate::offsetstore::OffsetStore;
use crate::result::{LogError, LogResult};
use crate::segment::Segment;

const OFFSET_ID: u64 = 0xffffffffffffffff;

/// Append only log.
///
/// This log is implemented based on the [Kafka log](http://notes.stephenholiday.com/Kafka.pdf)
/// (Section 3.2).
///
/// Each log contains a set of append only segments of a configured maximum
/// size. Records are accessed using their offset in the log.
///
/// TODO(AD) Support:
/// * Flush/sync (sync_all)
/// TODO rewrite
pub struct Log<S> {
    offsets: OffsetStore<S>,
    active: u64,
    segments: HashMap<u64, Box<S>>,
    segment_limit: u64,
    dir: PathBuf,
}

impl<S> Log<S>
where
    S: Segment,
{
    /// Creates a `Log` in the given directory.
    ///
    /// This will load all segments stored in `dir` and the offset table if
    /// exists.
    ///
    /// `segment_limit` is the maximum size of the segments.
    ///
    /// # Errors
    ///
    /// If the file system cannot be accessed returns `LogError::IoError`.
    pub async fn open(dir: &Path, segment_limit: u64) -> LogResult<Log<S>> {
        let seg = S::open(OFFSET_ID, dir).await.unwrap();
        let offsets = OffsetStore::<S>::new(seg).await.unwrap();
        let mut log = Log {
            offsets,
            active: 0,
            segments: HashMap::new(),
            segment_limit: segment_limit,
            dir: dir.to_path_buf(),
        };

        log.load_segments().await?;
        Ok(log)
    }

    /// Appends the given data to the log.
    ///
    /// If the active segment exceeds its configured max size a new segment
    /// is created.
    ///
    /// # Errors
    ///
    /// If the file system cannot be accessed returns `LogError::IoError`.
    pub async fn append(&mut self, data: &Vec<u8>) -> LogResult<()> {
        if let Some(seg) = self.segments.get_mut(&self.active) {
            seg.append(data).await?;
            let size = seg.size().await?;
            if self.is_segment_full(size) {
                // TODO (AD) Go back to append return offset to avoid seek
                // TODO Race condition? Need to be careful as multi threads operating on same data.
                // Limit to just one?
                self.extend_segments(size + self.offsets.max_offset())
                    .await?;
            }
        } else {
            // This can never happen so just crash.
            panic!("no active segment");
        }
        Ok(())
    }

    /// Returns `size` bytes starting at `offset`.
    ///
    /// # Errors
    ///
    /// * If the file system cannot be accessed returns `LogError::IoError`,
    /// * If the segment has expired and been removed returns `LogError::SegmentExpired`,
    /// * If the offset exceeds the log size returns `LogError::Eof`.
    pub async fn lookup(&mut self, offset: u64, size: u64) -> LogResult<Vec<u8>> {
        // Offsets always start at 0 so get never returns None.
        let (id, segment_offset) = self.offsets.get(offset).unwrap();
        if let Some(segment) = self.segments.get_mut(&id) {
            segment.lookup(offset - segment_offset, size).await
        } else {
            // Occurs if segment expired and removed.
            Err(LogError::SegmentExpired)
        }
    }

    // TODO(AD)
    // /// Erases all segments last modified before `before`.
    // ///
    // /// This will never remove the active segment.
    // ///
    // /// # Errors
    // ///
    // /// If the file system cannot be accessed returns `LogError::IoError`.
    // pub fn expire(&mut self, before: SystemTime) -> LogResult<()> {
    // let mut expired = Vec::new();
    // for (id, segment) in self.segments.iter() {
    // if segment.modified()? < before && *id != self.active {
    // expired.push(*id);
    // // segment.remove()?;
    // }
    // }

    // for id in expired.iter() {
    // self.segments.remove(id);
    // }

    // Ok(())
    // }

    async fn load_segments(&mut self) -> LogResult<()> {
        for id in S::segments(&self.dir).await? {
            if id != OFFSET_ID {
                self.segments.insert(id, S::open(id, &self.dir).await?);
            }
        }

        if self.segments.is_empty() {
            self.new_segment(0).await?;
            self.offsets.insert(0, 0).await?;
        }

        Ok(())
    }

    async fn extend_segments(&mut self, offset: u64) -> LogResult<()> {
        self.active += 1;
        self.new_segment(self.active).await?;
        self.offsets.insert(offset, self.active).await?;
        Ok(())
    }

    async fn new_segment(&mut self, id: u64) -> LogResult<()> {
        self.segments.insert(id, S::open(id, &self.dir).await?);
        Ok(())
    }

    fn is_segment_full(&self, size: u64) -> bool {
        size > self.segment_limit
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use crate::inmemorysegment::InMemorySegment;

    #[tokio::test]
    async fn open_empty() {
        let mut log = Log::<InMemorySegment>::open(&rand_path(), 10)
            .await
            .unwrap();

        if let Err(LogError::Eof) = log.lookup(0, 4).await {
        } else {
            panic!("expected EOF");
        }
    }

    #[tokio::test]
    async fn open_load() {
        let path = rand_path();
        {
            let mut log = Log::<InMemorySegment>::open(&path, 3).await.unwrap();

            // Segment 0.
            log.append(&vec![1, 2, 3, 4]).await.unwrap();
            // Segment 1.
            log.append(&vec![5, 6, 7, 8]).await.unwrap();
            // Segment 2.
            log.append(&vec![7]).await.unwrap();
            log.append(&vec![8]).await.unwrap();
            log.append(&vec![9, 10]).await.unwrap();
            // // Segment 3.
            log.append(&vec![11, 12]).await.unwrap();
        }
        {
            let mut log = Log::<InMemorySegment>::open(&path, 3).await.unwrap();
            assert_eq!(vec![1, 2, 3, 4], log.lookup(0, 4).await.unwrap());
            assert_eq!(vec![5, 6, 7, 8], log.lookup(4, 4).await.unwrap());
            assert_eq!(vec![7, 8], log.lookup(8, 2).await.unwrap());
            assert_eq!(vec![9, 10], log.lookup(10, 2).await.unwrap());
        }
    }

    #[tokio::test]
    async fn write_multiple_segments() {
        let path = rand_path();
        {
            let mut log = Log::<InMemorySegment>::open(&path, 3).await.unwrap();

            // Segment 0.
            log.append(&vec![1, 2, 3, 4]).await.unwrap();
            // Segment 1.
            log.append(&vec![5, 6, 7, 8]).await.unwrap();
            // Segment 2.
            log.append(&vec![7]).await.unwrap();
            log.append(&vec![8]).await.unwrap();
            log.append(&vec![9, 10]).await.unwrap();
            // // Segment 3.
            log.append(&vec![11, 12]).await.unwrap();
        }

        {
            // Check the records were written to different segments.
            let mut segment = InMemorySegment::open(0, &path).await.unwrap();
            assert_eq!(vec![1, 2, 3, 4], segment.lookup(0, 4).await.unwrap());
            let mut segment = InMemorySegment::open(1, &path).await.unwrap();
            assert_eq!(vec![5, 6, 7, 8], segment.lookup(0, 4).await.unwrap());
            let mut segment = InMemorySegment::open(2, &path).await.unwrap();
            assert_eq!(vec![7, 8], segment.lookup(0, 2).await.unwrap());
            assert_eq!(vec![9, 10], segment.lookup(2, 2).await.unwrap());
            let mut segment = InMemorySegment::open(3, &path).await.unwrap();
            assert_eq!(vec![11, 12], segment.lookup(0, 2).await.unwrap());
        }

        {
            let mut log = Log::<InMemorySegment>::open(&path, 3).await.unwrap();
            assert_eq!(vec![1, 2, 3, 4], log.lookup(0, 4).await.unwrap());
            assert_eq!(vec![5, 6, 7, 8], log.lookup(4, 4).await.unwrap());
            assert_eq!(vec![7, 8], log.lookup(8, 2).await.unwrap());
            assert_eq!(vec![9, 10], log.lookup(10, 2).await.unwrap());
        }
    }

    // #[test]
    // fn expire_active_segment() {
    // let mut manager: std::boxed::Box<(dyn SegmentManager + 'static)> =
    // Box::new(InMemorySegmentManager::new().unwrap());
    // let mut log = Log::open(&mut manager, 10).unwrap();

    // // Segment 0.
    // log.append(&vec![1, 2, 3, 4]).unwrap();

    // // Should NOT remove the active segment.
    // log.expire(SystemTime::now()).unwrap();

    // assert_eq!(vec![1, 2, 3, 4], log.lookup(4, 0).unwrap());
    /* } */

    fn rand_path() -> PathBuf {
        let mut path = PathBuf::new();
        path.push((0..0xf).map(|_| rand::random::<char>()).collect::<String>());
        path
    }
}
