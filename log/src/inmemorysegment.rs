use std::cell::RefCell;
use std::rc::Rc;
use std::time::SystemTime;
use std::vec::Vec;

use crate::result::{LogError, LogResult};
use crate::segment::Segment;

pub struct InMemorySegment {
    data: Rc<RefCell<Vec<u8>>>,
    modified: SystemTime,
}

impl InMemorySegment {
    pub fn new(data: Rc<RefCell<Vec<u8>>>) -> InMemorySegment {
        InMemorySegment {
            data,
            modified: SystemTime::now(),
        }
    }
}

impl Segment for InMemorySegment {
    fn append(&mut self, data: &Vec<u8>) -> LogResult<u64> {
        self.data.borrow_mut().append(&mut data.clone());
        self.modified = SystemTime::now();
        Ok(self.data.borrow_mut().len() as u64)
    }

    fn lookup(&mut self, size: u64, offset: u64) -> LogResult<Vec<u8>> {
        if size + offset > self.data.borrow_mut().len() as u64 {
            Err(LogError::Eof)
        } else {
            Ok(self.data.borrow_mut()[offset as usize..(size + offset) as usize].to_vec())
        }
    }

    fn modified(&self) -> LogResult<SystemTime> {
        Ok(self.modified)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn lookup_not_empty() {
        let data = Rc::new(RefCell::new(Vec::new()));
        let mut segment = InMemorySegment::new(data);

        segment.append(&vec![1, 2, 3]).unwrap();
        segment.append(&vec![4, 5, 6]).unwrap();

        assert_eq!(vec![1, 2, 3], segment.lookup(3, 0).unwrap());
        assert_eq!(vec![4], segment.lookup(1, 3).unwrap());
        assert_eq!(vec![5, 6], segment.lookup(2, 4).unwrap());
    }

    #[test]
    fn lookup_empty() {
        let data = Rc::new(RefCell::new(Vec::new()));
        let mut segment = InMemorySegment::new(data);
        if let Err(LogError::Eof) = segment.lookup(4, 0) {
        } else {
            panic!("expected segment expired");
        }
    }
}
