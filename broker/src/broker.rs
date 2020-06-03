use crate::result::BrokerResult;
use std::collections::HashMap;
use std::path::Path;
use wombatlog::SystemLog;
use wombatpartition::{Partition, PartitionID};

/// Handles looking up partitions to write to and consume from.
///
/// TODO(AD) Support:
/// * Coordination to assign partitions
/// * Async
pub struct Broker {
    partitions: HashMap<String, Partition<SystemLog>>,
    dir: String,
}

// TODO(AD) Background thread to expire old logs.
impl Broker {
    // TODO(AD) Should be able to use in memory log here.
    pub fn new(dir: &Path) -> BrokerResult<Broker> {
        Ok(Broker {
            partitions: HashMap::new(),
            dir: dir.to_str().unwrap().to_string(),
        })
    }

    pub fn consume(
        &mut self,
        partition: &PartitionID,
        offset: u64,
    ) -> BrokerResult<(Vec<u8>, u64)> {
        if !self.partitions.contains_key(&partition.to_string()) {
            // TODO(AD) DUP
            self.partitions.insert(
                partition.to_string(),
                Partition::new(SystemLog::new(
                    &Path::new(&self.dir).join(partition.to_string()),
                    1_000_000_000,
                )?),
            );
        }

        Ok(self
            .partitions
            .get_mut(&partition.to_string())
            .unwrap()
            .get(offset)?)
    }

    pub fn produce(&mut self, partition: &PartitionID, data: &Vec<u8>) -> BrokerResult<()> {
        if !self.partitions.contains_key(&partition.to_string()) {
            // TODO(AD) DUP
            self.partitions.insert(
                partition.to_string(),
                Partition::new(SystemLog::new(
                    &Path::new(&self.dir).join(partition.to_string()),
                    1_000_000_000,
                )?),
            );
        }

        Ok(self
            .partitions
            .get_mut(&partition.to_string())
            .unwrap()
            .put(data)?)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use tempdir::TempDir;

    #[test]
    fn empty() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut broker = Broker::new(tmp.path()).unwrap();

        let id = PartitionID::new("mytopic".to_string(), 0xf8a2);

        let written = vec![1, 2, 3];
        broker.produce(&id, &written).unwrap();

        let (read, _next) = broker.consume(&id, 0).unwrap();
        assert_eq!(written, read);
    }

    #[test]
    fn read_existing() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut broker = Broker::new(tmp.path()).unwrap();

        let id = PartitionID::new("mytopic".to_string(), 0xf8a2);

        let written = vec![1, 2, 3];
        broker.produce(&id, &written).unwrap();

        let mut broker = Broker::new(tmp.path()).unwrap();
        let (read, _next) = broker.consume(&id, 0).unwrap();
        assert_eq!(written, read);
    }

    #[test]
    fn multi_partitions() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut broker = Broker::new(tmp.path()).unwrap();

        let id1 = PartitionID::new("mytopic".to_string(), 0xf8a2);
        let id2 = PartitionID::new("mytopicother".to_string(), 0x2474);
        let id3 = PartitionID::new("mytopic".to_string(), 0x1234);

        broker.produce(&id1, &vec![1, 2, 3]).unwrap();
        broker.produce(&id2, &vec![4, 5, 6]).unwrap();
        broker.produce(&id3, &vec![7, 8, 9]).unwrap();

        let mut broker = Broker::new(tmp.path()).unwrap();
        let (read, _next) = broker.consume(&id1, 0).unwrap();
        assert_eq!(vec![1, 2, 3], read);

        let (read, _next) = broker.consume(&id2, 0).unwrap();
        assert_eq!(vec![4, 5, 6], read);

        let (read, _next) = broker.consume(&id3, 0).unwrap();
        assert_eq!(vec![7, 8, 9], read);
    }

    #[test]
    fn return_offset() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut broker = Broker::new(tmp.path()).unwrap();

        let id = PartitionID::new("mytopic".to_string(), 0xf8a2);

        broker.produce(&id, &vec![1, 2, 3]).unwrap();
        broker.produce(&id, &vec![4, 5, 6]).unwrap();
        broker.produce(&id, &vec![7, 8, 9]).unwrap();

        let mut broker = Broker::new(tmp.path()).unwrap();
        let (read, next) = broker.consume(&id, 0).unwrap();
        assert_eq!(vec![1, 2, 3], read);

        let (read, next) = broker.consume(&id, next).unwrap();
        assert_eq!(vec![4, 5, 6], read);

        let (read, _next) = broker.consume(&id, next).unwrap();
        assert_eq!(vec![7, 8, 9], read);
    }

    #[test]
    fn consume_eof() {
        let tmp = TempDir::new("log-unit-tests").unwrap();
        let mut broker = Broker::new(tmp.path()).unwrap();

        let id = PartitionID::new("mytopic".to_string(), 0xf8a2);

        if let Err(_) = broker.consume(&id, 0) {
        } else {
            panic!("expected EOF");
        }
    }
}
