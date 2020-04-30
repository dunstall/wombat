use std::collections::BTreeMap;

use crate::log::result::{Error, Result};
use crate::log::segment::Len;

pub struct Offsets<T: Len> {
    items: BTreeMap<u64, T>,
}

impl<T: Len> Offsets<T> {
    pub fn new() -> Offsets<T> {
        Offsets {
            items: BTreeMap::new(),
        }
    }

    pub fn add(&mut self, item: T) {
        self.items.insert(self.last_offset(), item);
    }

    pub fn active(&mut self) -> Result<&mut T> {
        match self.items.iter_mut().last() {
            Some((_, v)) => Ok(v),
            None => Err(Error::OffsetNotFound),
        }
    }

    /// Lookup the segment responsible for the given offset. If none exists returns an error.
    pub fn lookup(&mut self, offset: u64) -> Result<&mut T> {
        // Expected to be small so just iterate all. TODO(AD) Binary search.
        for (off, item) in self.items.iter_mut() {
            if *off <= offset && offset < *off + item.len() {
                return Ok(item);
            }
        }
        Err(Error::OffsetNotFound)
    }

    fn last_offset(&self) -> u64 {
        match self.items.iter().last() {
            Some((k, v)) => k + v.len(),
            None => 0,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::cmp::PartialEq;

    #[derive(Copy, Clone, Debug, PartialEq)]
    struct FakeItem {
        len: u64,
    }

    impl Len for FakeItem {
        fn len(&self) -> u64 {
            self.len
        }
    }

    #[test]
    fn add_to_empty() {
        let mut offsets: Offsets<FakeItem> = Offsets::new();
        let item = FakeItem { len: 1000 };

        offsets.add(item);

        assert_eq!(offsets.lookup(0).unwrap(), &item);
        assert_eq!(offsets.lookup(999).unwrap(), &item);
        assert_eq!(offsets.active().unwrap(), &item);

        // No item handles offset 1000.
        match offsets.lookup(1000) {
            Ok(_) => panic!("invalid lookup"),
            Err(_) => (),
        }
    }

    #[test]
    fn add_multi() {
        let mut offsets: Offsets<FakeItem> = Offsets::new();
        let item1 = FakeItem { len: 1000 };
        let item2 = FakeItem { len: 2000 };
        let item3 = FakeItem { len: 5000 };

        offsets.add(item1);
        assert_eq!(offsets.lookup(0).unwrap(), &item1);
        assert_eq!(offsets.lookup(999).unwrap(), &item1);
        assert_eq!(offsets.active().unwrap(), &item1);
        match offsets.lookup(1000) {
            Ok(_) => panic!("invalid lookup"),
            Err(_) => (),
        }

        offsets.add(item2);
        assert_eq!(offsets.lookup(1000).unwrap(), &item2);
        assert_eq!(offsets.lookup(2999).unwrap(), &item2);
        assert_eq!(offsets.active().unwrap(), &item2);
        match offsets.lookup(3000) {
            Ok(_) => panic!("invalid lookup"),
            Err(_) => (),
        }

        offsets.add(item3);
        assert_eq!(offsets.lookup(3000).unwrap(), &item3);
        assert_eq!(offsets.lookup(7999).unwrap(), &item3);
        assert_eq!(offsets.active().unwrap(), &item3);
        match offsets.lookup(8000) {
            Ok(_) => panic!("invalid lookup"),
            Err(_) => (),
        }
    }
}
