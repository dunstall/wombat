use regex::Regex;
use std::collections::HashMap;
use std::fs;
use std::path::Path;
use std::time::SystemTime;
use std::vec::Vec;

use crate::offsetstore::OffsetStore;
use crate::result::{LogError, LogResult};
use crate::segment::Segment;

/// Append only log.
///
/// This log is implemented based on the [Kafka log](http://notes.stephenholiday.com/Kafka.pdf)
/// (Section 3.2).
///
/// Each log contains a set of append only segments of a configured maximum
/// size. Records are accessed using their offset in the log.
pub struct Log {
    offsets: OffsetStore,
    active: u64,
    segments: HashMap<u64, Segment>,
    segment_limit: u64,
    dir: String,
}

impl Log {
    /// Creates a `Log` in the given directory.
    ///
    /// This will load all segments stored in `dir` and the offset table if
    /// exists.
    ///
    /// `segment_limit` is the maximum size of the segments.
    ///
    /// # Errors
    ///
    /// If the directory cannot be accessed returns an `Err`.
    pub fn new(dir: &Path, segment_limit: u64) -> LogResult<Log> {
        let mut log = Log {
            offsets: OffsetStore::new(dir)?,
            active: 0,
            segments: HashMap::new(),
            segment_limit,
            dir: dir.to_str().unwrap().to_string(),
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
    /// If the file system cannot be accessed.
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
    /// TODO none if eof or segment expired
    pub fn lookup(&mut self, size: u64, offset: u64) -> LogResult<Vec<u8>> {
        if let Some(segment) = self.offsets.get(offset) {
            if let Some(seg) = self.segments.get_mut(&segment.0) {
                seg.lookup(size, offset - segment.1)
            } else {
                // Occurs if segment expired and removed.
                Err(LogError::SegmentExpired)
            }
        } else {
            Err(LogError::OffsetNotFound)
        }
    }

    /// Erases all segments last modified before `before`.
    ///
    /// This will never remove the active segment.
    ///
    /// # Errors
    ///
    /// If the file system cannot be accessed.
    pub fn expire(&mut self, before: SystemTime) -> LogResult<()> {
        let mut expired = Vec::new();
        for (segment, _) in self.segments.iter() {
            let path = &Path::new(&self.dir).join(Log::segment_to_string(*segment));
            let metadata = fs::metadata(path)?;
            if metadata.modified()? < before && *segment != self.active {
                expired.push(*segment);
            }
        }

        for segment in expired.iter() {
            println!("expire {}", segment);
            self.segments.remove(segment);
            let path = &Path::new(&self.dir).join(Log::segment_to_string(*segment));
            fs::remove_file(path)?;
        }

        Ok(())
    }

    fn segment_to_string(segment: u64) -> String {
        format!("segment-{:0>8}", segment.to_string())
    }

    fn string_to_segment(s: &String) -> Option<u64> {
        let re = Regex::new(r"^segment-(\d{8})$").unwrap();
        if let Some(caps) = re.captures(s) {
            if let Some(n) = caps.get(1) {
                Some(n.as_str().parse::<u64>().unwrap()) // TODO
            } else {
                // TODO error?
                None
            }
        } else {
            None
        }
    }

    fn load_segments(&mut self) -> LogResult<()> {
        for entry in fs::read_dir(&self.dir)? {
            let entry = entry?;
            let name = entry.file_name().into_string()?;
            self.load_segment(name)?;
        }

        if self.segments.len() == 0 {
            self.new_segment(0)?;
            self.offsets.insert(0, 0)?;
        }

        Ok(())
    }

    fn load_segment(&mut self, file: String) -> LogResult<()> {
        if let Some(segment) = Log::string_to_segment(&file) {
            self.segments
                .insert(segment, Segment::new(&Path::new(&self.dir).join(file))?);
        }

        Ok(())
    }

    fn extend_segments(&mut self, offset: u64) -> LogResult<()> {
        // let offset = size + self.offsets.max_offset();
        self.active += 1;
        self.new_segment(self.active)?;
        self.offsets.insert(offset, self.active)?;
        Ok(())
    }

    fn new_segment(&mut self, segment: u64) -> LogResult<()> {
        self.segments.insert(
            segment,
            Segment::new(&Path::new(&self.dir).join(Log::segment_to_string(segment)))?,
        );
        Ok(())
    }

    fn is_segment_full(&self, size: u64) -> bool {
        size > self.segment_limit
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::thread::sleep;
    use std::time::Duration;
    use tempdir::TempDir;

    #[test]
    fn empty_log() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut log = Log::new(tmp.path(), 10).unwrap();

        let written = vec![1, 2, 3];
        log.append(&written).unwrap();

        let read = log.lookup(3, 0).unwrap();
        assert_eq!(written, read);
    }

    #[test]
    fn read_existing_single_segment() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut log = Log::new(tmp.path(), 3).unwrap();

        let written = vec![1, 2, 3];
        log.append(&written).unwrap();

        let mut log = Log::new(tmp.path(), 10).unwrap();
        let read = log.lookup(3, 0).unwrap();
        assert_eq!(written, read);
    }

    #[test]
    fn write_multiple_segments() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut log = Log::new(tmp.path(), 3).unwrap();

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

        // Check the records were written to different segments.
        let mut segment = Segment::new(&tmp.path().join("segment-00000000")).unwrap();
        assert_eq!(vec![1, 2, 3, 4], segment.lookup(4, 0).unwrap());
        let mut segment = Segment::new(&tmp.path().join("segment-00000001")).unwrap();
        assert_eq!(vec![5, 6, 7, 8], segment.lookup(4, 0).unwrap());
        let mut segment = Segment::new(&tmp.path().join("segment-00000002")).unwrap();
        assert_eq!(vec![7, 8], segment.lookup(2, 0).unwrap());
        assert_eq!(vec![9, 10], segment.lookup(2, 2).unwrap());
        let mut segment = Segment::new(&tmp.path().join("segment-00000003")).unwrap();
        assert_eq!(vec![11, 12], segment.lookup(2, 0).unwrap());

        assert_eq!(vec![1, 2, 3, 4], log.lookup(4, 0).unwrap());
        assert_eq!(vec![5, 6, 7, 8], log.lookup(4, 4).unwrap());
        assert_eq!(vec![7, 8], log.lookup(2, 8).unwrap());
        assert_eq!(vec![9, 10], log.lookup(2, 10).unwrap());
    }

    #[test]
    fn load_log() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut log = Log::new(tmp.path(), 3).unwrap();

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

        let mut log = Log::new(tmp.path(), 3).unwrap();
        assert_eq!(vec![1, 2, 3, 4], log.lookup(4, 0).unwrap());
        assert_eq!(vec![5, 6, 7, 8], log.lookup(4, 4).unwrap());
        assert_eq!(vec![7, 8], log.lookup(2, 8).unwrap());
        assert_eq!(vec![9, 10], log.lookup(2, 10).unwrap());
    }

    #[test]
    fn expire_active_segment() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut log = Log::new(tmp.path(), 10).unwrap();

        // Segment 0.
        log.append(&vec![1, 2, 3, 4]).unwrap();

        // Should NOT remove the active segment.
        log.expire(SystemTime::now()).unwrap();

        assert_eq!(vec![1, 2, 3, 4], log.lookup(4, 0).unwrap());
    }

    #[test]
    fn expire_old_segment() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut log = Log::new(tmp.path(), 3).unwrap();

        // Fill the active segment to create new.
        log.append(&vec![1, 2, 3, 4]).unwrap();
        let exp = SystemTime::now();
        log.append(&vec![5, 6]).unwrap(); // Active segment.

        sleep(Duration::new(1, 0));

        log.expire(exp).unwrap();

        if let Err(LogError::SegmentExpired) = log.lookup(4, 0) {
        } else {
            panic!("expected segment expired");
        }

        assert_eq!(vec![5, 6], log.lookup(2, 4).unwrap());
    }

    #[test]
    fn load_log_with_expired_segments() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut log = Log::new(tmp.path(), 3).unwrap();

        // Fill the active segment to create new.
        log.append(&vec![1, 2, 3, 4]).unwrap();
        let exp = SystemTime::now();
        log.append(&vec![5, 6]).unwrap(); // Active segment.

        sleep(Duration::new(1, 0));

        log.expire(exp).unwrap();

        // Load the log again.
        let mut log = Log::new(tmp.path(), 3).unwrap();

        if let Err(LogError::SegmentExpired) = log.lookup(4, 0) {
        } else {
            panic!("expected segment expired");
        }

        assert_eq!(vec![5, 6], log.lookup(2, 4).unwrap());
    }

    #[test]
    fn lookup_eof() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut log = Log::new(tmp.path(), 3).unwrap();

        if let Err(LogError::Eof) = log.lookup(4, 0) {
        } else {
            panic!("expected segment expired");
        }
    }
}
