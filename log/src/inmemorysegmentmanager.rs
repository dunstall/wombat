use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

use crate::inmemorysegment::InMemorySegment;
use crate::result::LogResult;
use crate::segment;
use crate::segment::Segment;
use crate::segmentmanager::SegmentManager;

pub struct InMemorySegmentManager {
    segments: HashMap<String, Rc<RefCell<Vec<u8>>>>,
}

impl InMemorySegmentManager {
    pub fn new() -> LogResult<InMemorySegmentManager> {
        Ok(InMemorySegmentManager {
            segments: HashMap::new(),
        })
    }
}

impl SegmentManager for InMemorySegmentManager {
    fn open(&mut self, id: u64) -> LogResult<Box<dyn Segment>> {
        if let Some(data) = self.segments.get(&segment::id_to_name(id)) {
            let segment = Box::new(InMemorySegment::new(data.clone()));
            Ok(segment)
        } else {
            let data = Rc::new(RefCell::new(Vec::<u8>::new()));
            let segment = Box::new(InMemorySegment::new(data.clone()));
            self.segments.insert(segment::id_to_name(id), data);
            Ok(segment)
        }
    }

    fn open_name(&mut self, name: String) -> LogResult<Box<dyn Segment>> {
        if let Some(data) = self.segments.get(&name) {
            let segment = Box::new(InMemorySegment::new(data.clone()));
            Ok(segment)
        } else {
            let data = Rc::new(RefCell::new(Vec::<u8>::new()));
            let segment = Box::new(InMemorySegment::new(data.clone()));
            self.segments.insert(name, data);
            Ok(segment)
        }
    }

    fn remove(&mut self, id: u64) -> LogResult<()> {
        if let Some(data) = self.segments.get_mut(&segment::id_to_name(id)) {
            data.replace(vec![]);
        }
        self.segments.remove(&segment::id_to_name(id));
        Ok(())
    }

    fn segments(&self) -> LogResult<Vec<u64>> {
        let mut ids = Vec::new();
        for name in self.segments.keys() {
            if let Some(id) = segment::name_to_id(name) {
                ids.push(id);
            }
        }
        Ok(ids)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use crate::result::LogError;

    #[test]
    fn open_segment_not_exists() {
        let mut manager = InMemorySegmentManager::new().unwrap();
        manager.open(0xaabb).unwrap();
    }

    #[test]
    fn open_segment_exists() {
        let mut manager = InMemorySegmentManager::new().unwrap();
        manager
            .open(0xaabb)
            .unwrap()
            .append(&vec![1, 2, 3])
            .unwrap();
        // Re-open existing
        assert_eq!(
            vec![1, 2, 3],
            manager.open(0xaabb).unwrap().lookup(3, 0).unwrap()
        );
    }

    #[test]
    fn remove_segment_not_exists() {
        let mut manager = InMemorySegmentManager::new().unwrap();
        manager.remove(0xaabb).unwrap();
    }

    #[test]
    fn remove_segment_exists() {
        let mut manager = InMemorySegmentManager::new().unwrap();
        let mut seg = manager.open(0xaabb).unwrap();
        seg.append(&vec![1, 2, 3]).unwrap();
        manager.remove(0xaabb).unwrap();

        // Re-open the segment which should be empty.
        let mut seg = manager.open(0xaabb).unwrap();
        if let Err(LogError::Eof) = seg.lookup(3, 0) {
        } else {
            panic!("expected EOF");
        }
    }

    #[test]
    fn segments_empty() {
        let manager = InMemorySegmentManager::new().unwrap();
        assert_eq!(true, manager.segments().unwrap().is_empty());
    }

    #[test]
    fn segments_not_empty() {
        let mut manager = InMemorySegmentManager::new().unwrap();
        manager.open(0xaabb).unwrap();
        manager.open(0xccdd).unwrap();
        let mut s = manager.segments().unwrap();
        s.sort();
        assert_eq!(vec![0xaabb, 0xccdd], s);
    }
}
