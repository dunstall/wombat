use std::collections::HashMap;
use std::time::SystemTime;
use std::vec::Vec;

use crate::offsetstore::OffsetStore;
use crate::result::{LogError, LogResult};
use crate::segment::Segment;
use crate::segmentmanager::SegmentManager;

/// Append only log.
///
/// This log is implemented based on the [Kafka log](http://notes.stephenholiday.com/Kafka.pdf)
/// (Section 3.2).
///
/// Each log contains a set of append only segments of a configured maximum
/// size. Records are accessed using their offset in the log.
///
/// TODO(AD) Support:
/// * Async
/// * Flush/sync (sync_all)
pub struct Log<'a> {
    offsets: OffsetStore,
    active: u64,
    segments: HashMap<u64, Box<dyn Segment>>,
    segment_limit: u64,
    segment_manager: &'a mut Box<dyn SegmentManager>,
}

impl<'a> Log<'a> {
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
    pub fn open(
        segment_manager: &'a mut Box<dyn SegmentManager>,
        segment_limit: u64,
    ) -> LogResult<Log> {
        let segment = segment_manager.open_name("segment.offsets".to_string())?;
        let offsets = OffsetStore::new(segment)?;

        let mut log = Log {
            segment_manager,
            offsets,
            active: 0,
            segments: HashMap::new(),
            segment_limit: segment_limit,
        };

        log.load_segments()?;
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
    pub fn append(&mut self, data: &Vec<u8>) -> LogResult<()> {
        if let Some(seg) = self.segments.get_mut(&self.active) {
            let size = seg.append(data)?;
            if self.is_segment_full(size) {
                self.extend_segments(size + self.offsets.max_offset())?;
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
    pub fn lookup(&mut self, size: u64, offset: u64) -> LogResult<Vec<u8>> {
        // Offsets always start at 0 so get never returns None.
        let (id, segment_offset) = self.offsets.get(offset).unwrap();
        if let Some(segment) = self.segments.get_mut(&id) {
            segment.lookup(size, offset - segment_offset)
        } else {
            // Occurs if segment expired and removed.
            Err(LogError::SegmentExpired)
        }
    }

    /// Erases all segments last modified before `before`.
    ///
    /// This will never remove the active segment.
    ///
    /// # Errors
    ///
    /// If the file system cannot be accessed returns `LogError::IoError`.
    pub fn expire(&mut self, before: SystemTime) -> LogResult<()> {
        let mut expired = Vec::new();
        for (id, segment) in self.segments.iter() {
            if segment.modified()? < before && *id != self.active {
                expired.push(*id);
                // segment.remove()?;
            }
        }

        for id in expired.iter() {
            self.segments.remove(id);
        }

        Ok(())
    }

    fn load_segments(&mut self) -> LogResult<()> {
        for id in self.segment_manager.segments()? {
            self.segments.insert(id, self.segment_manager.open(id)?);
            // TODO must insert into offsets?
        }

        if self.segments.is_empty() {
            self.new_segment(0)?;
            self.offsets.insert(0, 0)?;
        }

        Ok(())
    }

    fn extend_segments(&mut self, offset: u64) -> LogResult<()> {
        self.active += 1;
        self.new_segment(self.active)?;
        self.offsets.insert(offset, self.active)?;
        Ok(())
    }

    fn new_segment(&mut self, id: u64) -> LogResult<()> {
        self.segments.insert(id, self.segment_manager.open(id)?);
        Ok(())
    }

    fn is_segment_full(&self, size: u64) -> bool {
        size > self.segment_limit
        // false
    }
}
// TODO rewrite and tdd

#[cfg(test)]
mod tests {
    use super::*;

    use crate::inmemorysegmentmanager::InMemorySegmentManager;

    #[test]
    fn open_empty() {
        let mut manager: std::boxed::Box<(dyn SegmentManager + 'static)> =
            Box::new(InMemorySegmentManager::new().unwrap());
        let mut log = Log::open(&mut manager, 10).unwrap();

        if let Err(LogError::Eof) = log.lookup(4, 0) {
        } else {
            panic!("expected EOF");
        }
    }

    #[test]
    fn open_load() {
        let mut manager: std::boxed::Box<(dyn SegmentManager + 'static)> =
            Box::new(InMemorySegmentManager::new().unwrap());
        {
            let mut log = Log::open(&mut manager, 10).unwrap();

            // Segment 0.
            log.append(&vec![1, 2, 3, 4]).unwrap();
            // Segment 1.
            log.append(&vec![5, 6, 7, 8]).unwrap();
            // Segment 2.
            log.append(&vec![7]).unwrap();
            log.append(&vec![8]).unwrap();
            log.append(&vec![9, 10]).unwrap();
            // // Segment 3.
            log.append(&vec![11, 12]).unwrap();
        }
        {
            let mut log = Log::open(&mut manager, 10).unwrap();
            assert_eq!(vec![1, 2, 3, 4], log.lookup(4, 0).unwrap());
            assert_eq!(vec![5, 6, 7, 8], log.lookup(4, 4).unwrap());
            assert_eq!(vec![7, 8], log.lookup(2, 8).unwrap());
            assert_eq!(vec![9, 10], log.lookup(2, 10).unwrap());
        }
    }

    #[test]
    fn write_multiple_segments() {
        let mut manager: std::boxed::Box<(dyn SegmentManager + 'static)> =
            Box::new(InMemorySegmentManager::new().unwrap());
        {
            let mut log = Log::open(&mut manager, 3).unwrap();

            // Segment 0.
            log.append(&vec![1, 2, 3, 4]).unwrap();
            // Segment 1.
            log.append(&vec![5, 6, 7, 8]).unwrap();
            // Segment 2.
            log.append(&vec![7]).unwrap();
            log.append(&vec![8]).unwrap();
            log.append(&vec![9, 10]).unwrap();
            // // Segment 3.
            log.append(&vec![11, 12]).unwrap();
        }

        {
            // Check the records were written to different segments.
            let mut segment = manager.open(0).unwrap();
            assert_eq!(vec![1, 2, 3, 4], segment.lookup(4, 0).unwrap());
            let mut segment = manager.open(1).unwrap();
            assert_eq!(vec![5, 6, 7, 8], segment.lookup(4, 0).unwrap());
            let mut segment = manager.open(2).unwrap();
            assert_eq!(vec![7, 8], segment.lookup(2, 0).unwrap());
            assert_eq!(vec![9, 10], segment.lookup(2, 2).unwrap());
            let mut segment = manager.open(3).unwrap();
            assert_eq!(vec![11, 12], segment.lookup(2, 0).unwrap());
        }

        {
            let mut log = Log::open(&mut manager, 3).unwrap();
            assert_eq!(vec![1, 2, 3, 4], log.lookup(4, 0).unwrap());
            assert_eq!(vec![5, 6, 7, 8], log.lookup(4, 4).unwrap());
            assert_eq!(vec![7, 8], log.lookup(2, 8).unwrap());
            assert_eq!(vec![9, 10], log.lookup(2, 10).unwrap());
        }
    }

    #[test]
    fn expire_active_segment() {
        let mut manager: std::boxed::Box<(dyn SegmentManager + 'static)> =
            Box::new(InMemorySegmentManager::new().unwrap());
        let mut log = Log::open(&mut manager, 10).unwrap();

        // Segment 0.
        log.append(&vec![1, 2, 3, 4]).unwrap();

        // Should NOT remove the active segment.
        log.expire(SystemTime::now()).unwrap();

        assert_eq!(vec![1, 2, 3, 4], log.lookup(4, 0).unwrap());
    }
}
