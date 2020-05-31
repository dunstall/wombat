use regex::Regex;
use std::collections::HashMap;
use std::fs;
use std::path::Path;
use std::vec::Vec;

use crate::offsetstore::OffsetStore;
use crate::result::{LogError, LogResult};
use crate::segment::Segment;

pub struct Log {
    offsets: OffsetStore,
    active: u64,
    segments: HashMap<u64, Segment>,
    seg_size: u64,
    dir: String,
}

impl Log {
    pub fn new(dir: &Path, seg_size: u64) -> LogResult<Log> {
        let mut log = Log {
            offsets: OffsetStore::new(&dir.join("offsets"))?,
            active: 0,
            segments: HashMap::new(),
            seg_size,
            dir: dir.to_str().unwrap().to_string(),
        };
        log.load_segments()?;
        Ok(log)
    }

    pub fn append(&mut self, data: &Vec<u8>) -> LogResult<()> {
        if let Some(seg) = self.segments.get_mut(&self.active) {
            let size = seg.append(data)?;
            if size > self.seg_size {
                let new_off = size + self.offsets.max_offset() as u64;
                self.active += 1;
                self.new_segment(self.active)?;
                self.offsets.insert(new_off as usize, self.active)?;
            }
        } else {
            // This can never happen to just crash.
            panic!("no active segment");
        }
        Ok(())
    }

    pub fn lookup(&mut self, size: usize, offset: usize) -> LogResult<Vec<u8>> {
        if let Some(segment) = self.offsets.get(offset) {
            if let Some(seg) = self.segments.get_mut(&segment.0) {
                seg.lookup(size, offset - segment.1 as usize)
            } else {
                // This can never happen to just crash.
                panic!("tracked segment not found");
            }
        } else {
            Err(LogError::OffsetNotFound)
        }
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

    fn new_segment(&mut self, segment: u64) -> LogResult<()> {
        self.segments.insert(
            segment,
            Segment::new(&Path::new(&self.dir).join(Log::segment_to_string(segment)))?,
        );
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

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
    fn write_exceeds_segment_length() {
        // TODO
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

        let mut segment = Segment::new(&tmp.path().join("segment-00000000")).unwrap();
        assert_eq!(vec![1, 2, 3, 4], segment.lookup(4, 0).unwrap());

        let mut segment = Segment::new(&tmp.path().join("segment-00000001")).unwrap();
        assert_eq!(vec![5, 6, 7, 8], segment.lookup(4, 0).unwrap());

        let mut segment = Segment::new(&tmp.path().join("segment-00000002")).unwrap();
        assert_eq!(vec![7, 8], segment.lookup(2, 0).unwrap());
        assert_eq!(vec![9, 10], segment.lookup(2, 2).unwrap());

        let mut segment = Segment::new(&tmp.path().join("segment-00000003")).unwrap();
        assert_eq!(vec![11, 12], segment.lookup(2, 0).unwrap());

        let mut log = Log::new(tmp.path(), 3).unwrap();
        assert_eq!(vec![1, 2, 3, 4], log.lookup(4, 0).unwrap());
        assert_eq!(vec![5, 6, 7, 8], log.lookup(4, 4).unwrap());
        assert_eq!(vec![7, 8], log.lookup(2, 8).unwrap());
        assert_eq!(vec![9, 10], log.lookup(2, 10).unwrap());
    }

    // #[test]
    // fn read_existing_multi_segment() { TODO
    // let tmp = TempDir::new("log-unit-tests").unwrap();
    // let mut log = Log::new(tmp.path(), 100).unwrap();

    // let written = vec![1, 2, 3];
    // log.append(&written).unwrap();

    // let mut log = Log::new(tmp.path(), 10).unwrap();
    // let read = log.lookup(3, 0).unwrap();
    // assert_eq!(written, read);
    // }

    // TODO write many

    // #[test]
    // fn read_existing_multi_segment() {
    // let tmp = TempDir::new("log-unit-tests").unwrap();

    // {
    // let mut segment = Segment::new(&tmp.path().join("segment-0000")).unwrap();

    // let written = vec![1, 2, 3];
    // assert_eq!(segment.append(&written).unwrap(), 3);
    // }
    // {
    // let mut segment = Segment::new(&tmp.path().join("segment-0001")).unwrap();

    // let written = vec![4, 5, 6];
    // assert_eq!(segment.append(&written).unwrap(), 3);
    // }

    // {
    // let mut segment = Segment::new(&tmp.path().join("segment-0002")).unwrap();

    // let written = vec![7, 8, 9];
    // assert_eq!(segment.append(&written).unwrap(), 3);
    // }

    // let mut log = Log::new(tmp.path(), 10).unwrap();
    // let read = log.lookup(3, 0).unwrap();
    // assert_eq!(vec![1, 2, 3], read);

    // let read = log.lookup(3, 3).unwrap();
    // assert_eq!(vec![4, 5, 6], read);

    // let read = log.lookup(3, 6).unwrap();
    // assert_eq!(vec![7, 8, 9], read);
    /* } */

    // TODO segment not starting at 0 (create log then remove segment)

    // TODO load offsets etc - create full multi seg log and reload

    // TODO write to segments at diff paths and check their loaded
}
