use crate::result::BrokerResult;
use std::collections::HashMap;
use std::path::{Path, PathBuf};
// use wombatlog::SystemLog;
use wombatlog::Segment;
use wombatpartition::{Partition, PartitionID};

/// Handles looking up partitions to write to and consume from.
///
/// TODO(AD) Support:
/// * Coordination to assign partitions
/// * Async
pub struct Broker<S> {
    partitions: HashMap<String, Partition<S>>,
    dir: PathBuf,
}

// TODO(AD) Background thread to expire old logs.
impl<S> Broker<S>
where
    S: Segment,
{
    // TODO(AD) Should be able to use in memory log here.
    pub fn new(dir: &Path) -> BrokerResult<Broker<S>> {
        Ok(Broker {
            partitions: HashMap::new(),
            dir: dir.to_path_buf(),
        })
    }

    pub async fn consume(
        &mut self,
        partition: &PartitionID,
        offset: u64,
    ) -> BrokerResult<(Vec<u8>, u64)> {
        if !self.partitions.contains_key(&partition.to_string()) {
            // TODO(AD) DUP
            self.partitions.insert(
                partition.to_string(), // TODO just store ID not string
                Partition::new(&self.dir.join(partition.to_string())).await?,
            );
        }

        Ok(self
            .partitions
            .get_mut(&partition.to_string())
            .unwrap()
            .get(offset)
            .await?)
    }

    pub async fn produce(&mut self, partition: &PartitionID, data: &Vec<u8>) -> BrokerResult<()> {
        if !self.partitions.contains_key(&partition.to_string()) {
            // TODO(AD) DUP
            self.partitions.insert(
                partition.to_string(),
                Partition::new(&self.dir.join(partition.to_string())).await?,
            );
        }

        Ok(self
            .partitions
            .get_mut(&partition.to_string())
            .unwrap()
            .put(data)
            .await?)
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
        let mut broker = Broker::<InMemorySegment>::new(&path).unwrap();

        let id = PartitionID::new("mytopic".to_string(), 0xf8a2);

        let written = vec![1, 2, 3];
        broker.produce(&id, &written).await.unwrap();

        let (read, _next) = broker.consume(&id, 0).await.unwrap();
        assert_eq!(written, read);
    }

    #[tokio::test]
    async fn read_existing() {
        let path = rand_path();
        let mut broker = Broker::<InMemorySegment>::new(&path).unwrap();

        let id = PartitionID::new("mytopic".to_string(), 0xf8a2);

        let written = vec![1, 2, 3];
        broker.produce(&id, &written).await.unwrap();

        let mut broker = Broker::<InMemorySegment>::new(&path).unwrap();
        let (read, _next) = broker.consume(&id, 0).await.unwrap();
        assert_eq!(written, read);
    }

    #[tokio::test]
    async fn multi_partitions() {
        let path = rand_path();
        let mut broker = Broker::<InMemorySegment>::new(&path).unwrap();

        let id1 = PartitionID::new("mytopic".to_string(), 0xf8a2);
        let id2 = PartitionID::new("mytopicother".to_string(), 0x2474);
        let id3 = PartitionID::new("mytopic".to_string(), 0x1234);

        broker.produce(&id1, &vec![1, 2, 3]).await.unwrap();
        broker.produce(&id2, &vec![4, 5, 6]).await.unwrap();
        broker.produce(&id3, &vec![7, 8, 9]).await.unwrap();

        let mut broker = Broker::<InMemorySegment>::new(&path).unwrap();
        let (read, _next) = broker.consume(&id1, 0).await.unwrap();
        assert_eq!(vec![1, 2, 3], read);

        let (read, _next) = broker.consume(&id2, 0).await.unwrap();
        assert_eq!(vec![4, 5, 6], read);

        let (read, _next) = broker.consume(&id3, 0).await.unwrap();
        assert_eq!(vec![7, 8, 9], read);
    }

    #[tokio::test]
    async fn return_offset() {
        let path = rand_path();
        let mut broker = Broker::<InMemorySegment>::new(&path).unwrap();

        let id = PartitionID::new("mytopic".to_string(), 0xf8a2);

        broker.produce(&id, &vec![1, 2, 3]).await.unwrap();
        broker.produce(&id, &vec![4, 5, 6]).await.unwrap();
        broker.produce(&id, &vec![7, 8, 9]).await.unwrap();

        let mut broker = Broker::<InMemorySegment>::new(&path).unwrap();
        let (read, next) = broker.consume(&id, 0).await.unwrap();
        assert_eq!(vec![1, 2, 3], read);

        let (read, next) = broker.consume(&id, next).await.unwrap();
        assert_eq!(vec![4, 5, 6], read);

        let (read, _next) = broker.consume(&id, next).await.unwrap();
        assert_eq!(vec![7, 8, 9], read);
    }

    #[tokio::test]
    async fn consume_eof() {
        let path = rand_path();
        let mut broker = Broker::<InMemorySegment>::new(&path).unwrap();

        let id = PartitionID::new("mytopic".to_string(), 0xf8a2);

        if let Err(_) = broker.consume(&id, 0).await {
        } else {
            panic!("expected EOF");
        }
    }

    fn rand_path() -> PathBuf {
        let mut path = PathBuf::new();
        path.push((0..0xf).map(|_| rand::random::<char>()).collect::<String>());
        path
    }
}
