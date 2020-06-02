use byteorder::{BigEndian, ReadBytesExt, WriteBytesExt};
use std::io::Cursor;
use std::vec::Vec;
use wombatlog::Log;

use crate::result::PartitionResult;

/// Handles framing data to store in the log.
///
/// This allows records to be looked up by offset only (rather than offset and
/// size).
///
/// TODO(AD) Support:
/// * Async: Must be careful of interleaving size and data
/// * Replication
/// * CRC check
/// * Timestamp
pub struct Partition<L> {
    log: L,
}

impl<L> Partition<L>
where
    L: Log,
{
    /// Creates a `Partition` using the given log.
    pub fn new(log: L) -> Partition<L> {
        Partition { log: log }
    }

    /// Returns the data at the `offset` and the offset of the next
    /// record.
    ///
    /// # Errors
    ///
    /// If the record cannot be accessed returns `PartitionError::LogError`.
    pub fn get(&mut self, offset: u64) -> PartitionResult<(Vec<u8>, u64)> {
        let mut rdr = Cursor::new(self.log.lookup(8, offset)?);
        let len = rdr.read_u64::<BigEndian>()?;
        Ok((self.log.lookup(len, offset + 8)?, offset + 8 + len))
    }

    /// Writes `data` to the partition.
    ///
    /// # Errors
    ///
    /// If the record cannot be accessed returns `PartitionError::LogError`.
    pub fn put(&mut self, data: &Vec<u8>) -> PartitionResult<()> {
        let mut size = vec![];
        size.write_u64::<BigEndian>(data.len() as u64)?;
        self.log.append(&size)?;
        self.log.append(data)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use wombatlog::InMemoryLog;

    #[test]
    fn empty_log() {
        let mut partition = Partition::new(InMemoryLog::new());

        let written = vec![1, 2, 3];
        partition.put(&written).unwrap();

        let read = partition.get(0).unwrap();
        assert_eq!((written, 11), read);
    }

    #[test]
    fn write_multi() {
        let mut partition = Partition::new(InMemoryLog::new());

        partition.put(&vec![1, 2, 3]).unwrap();
        partition.put(&vec![4, 5, 6]).unwrap();
        partition.put(&vec![7, 8, 9]).unwrap();

        let (read, next) = partition.get(0).unwrap();
        assert_eq!(vec![1, 2, 3], read);

        let (read, next) = partition.get(next).unwrap();
        assert_eq!(vec![4, 5, 6], read);

        let (read, _next) = partition.get(next).unwrap();
        assert_eq!(vec![7, 8, 9], read);
    }
}
