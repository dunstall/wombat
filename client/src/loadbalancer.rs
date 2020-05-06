use crc::{crc32, Hasher32};
use std::collections::HashMap;

// TODO(AD) Configurable.
const N_PARTITIONS: u32 = 15;

pub struct LoadBalancer {
    next: HashMap<String, u32>,
}

impl LoadBalancer {
    pub fn new() -> LoadBalancer {
        LoadBalancer {
            next: HashMap::new(),
        }
    }

    // Returns the next partition for this topic - calculated in via round-
    // robin.
    pub fn next(&mut self, topic: &str) -> u32 {
        if let Some(next) = self.next.get_mut(topic) {
            *next = *next % N_PARTITIONS + 1;
            *next
        } else {
            self.next.insert(topic.to_string(), 1);
            1
        }
    }

    pub fn from_key(&self, key: &[u8]) -> u32 {
        let mut digest = crc32::Digest::new_with_initial(crc32::IEEE, 0);
        digest.write(key);
        digest.sum32() % N_PARTITIONS
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn next() {
        let mut lb = LoadBalancer::new();
        for n in 0..100 {
            assert_eq!(n % N_PARTITIONS + 1, lb.next("mytopic"));
        }
        for n in 0..100 {
            assert_eq!(n % N_PARTITIONS + 1, lb.next("mytopic2"));
        }
    }

    #[test]
    fn map_to_partition() {
        let mut lb = LoadBalancer::new();
        assert_eq!(0, lb.from_key(&vec![]));
        assert_eq!(2, lb.from_key(&vec![0xff, 0xab]));
        assert_eq!(5, lb.from_key(&vec![0xff, 0xaa]));
    }
}
