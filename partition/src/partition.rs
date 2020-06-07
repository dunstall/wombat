use byteorder::{BigEndian, ReadBytesExt, WriteBytesExt};
use std::io::Cursor;
use std::path::Path;
use std::vec::Vec;
use wombatlog::{Log, Segment};

use crate::result::PartitionResult;

const SEGMENT_MAX_SIZE: u64 = 1_000_000_000;

/// Handles framing data to store in the log.
///
/// This allows records to be looked up by offset only (rather than offset and
/// size).
///
/// TODO(AD) Support:
/// * Async: Must be careful of interleaving size and data TODO just only
///     allow one partition per thread
/// * Replication
/// * CRC check
/// * Timestamp
pub struct Partition<S> {
    log: Log<S>,
}

impl<S> Partition<S>
where
    S: Segment,
{
    /// Creates a `Partition` using the given log.
    pub async fn new(dir: &Path) -> PartitionResult<Partition<S>> {
        Ok(Partition {
            log: Log::<S>::open(dir, SEGMENT_MAX_SIZE).await?,
        })
    }

    /// Returns the data at the `offset` and the offset of the next
    /// record.
    ///
    /// # Errors
    ///
    /// If the record cannot be accessed returns `PartitionError::LogError`.
    pub async fn get(&mut self, offset: u64) -> PartitionResult<(Vec<u8>, u64)> {
        let mut rdr = Cursor::new(self.log.lookup(offset, 8).await?);
        let len = rdr.read_u64::<BigEndian>()?;
        Ok((self.log.lookup(offset + 8, len).await?, offset + 8 + len))
    }

    /// Writes `data` to the partition.
    ///
    /// # Errors
    ///
    /// If the record cannot be accessed returns `PartitionError::LogError`.
    pub async fn put(&mut self, data: &Vec<u8>) -> PartitionResult<()> {
        let mut size = vec![];
        size.write_u64::<BigEndian>(data.len() as u64)?;
        self.log.append(&size).await?;
        self.log.append(data).await?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::path::PathBuf;
    use wombatlog::InMemorySegment;

    #[tokio::test]
    async fn open_empty() {
        let path = rand_path();

        let mut partition = Partition::<InMemorySegment>::new(&path).await.unwrap();

        let written = vec![1, 2, 3];
        partition.put(&written).await.unwrap();

        let read = partition.get(0).await.unwrap();
        assert_eq!((written, 11), read);
    }

    #[tokio::test]
    async fn write_multi() {
        let path = rand_path();

        let mut partition = Partition::<InMemorySegment>::new(&path).await.unwrap();

        partition.put(&vec![1, 2, 3]).await.unwrap();
        partition.put(&vec![4, 5, 6]).await.unwrap();
        partition.put(&vec![7, 8, 9]).await.unwrap();

        let (read, next) = partition.get(0).await.unwrap();
        assert_eq!(vec![1, 2, 3], read);

        let (read, next) = partition.get(next).await.unwrap();
        assert_eq!(vec![4, 5, 6], read);

        let (read, _next) = partition.get(next).await.unwrap();
        assert_eq!(vec![7, 8, 9], read);
    }

    fn rand_path() -> PathBuf {
        let mut path = PathBuf::new();
        path.push((0..0xf).map(|_| rand::random::<char>()).collect::<String>());
        path
    }
}
