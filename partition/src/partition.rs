use std::vec::Vec;
use wombatlog::Log;

use crate::result::PartitionResult;

pub struct Partition<L> {
    log: L,
}

impl<L> Partition<L>
where
    L: Log,
{
    pub fn new(log: L) -> Partition<L> {
        Partition { log: log }
    }

    pub fn get(offset: u64) -> PartitionResult<Vec<u8>> {
        Ok(vec![])
    }

    pub fn put(data: &Vec<u8>) -> PartitionResult<Vec<u8>> {
        Ok(vec![])
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use wombatlog::InMemoryLog;

    #[test]
    fn new() {
        Partition::new(InMemoryLog::new());
    }
}
