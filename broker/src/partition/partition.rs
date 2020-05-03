use std::path::Path;

use crate::partition::log::Log;
use crate::partition::log::Record;

pub struct Partition {
    log: Log,
}

impl Partition {
    // TODO(AD) Remove unwrap() and clone()

    pub async fn open(dir: &Path, topic: &str, n: u32) -> Partition {
        let path = dir.join(topic).join(format!("partition-{}", n));
        Partition {
            log: Log::open(&path).await.unwrap(),
        }
    }

    pub fn map_to_partition(key: &Vec<u8>) -> u32 {
        0 // TODO(AD)
    }

    pub async fn get(&mut self, offset: u64) -> (u64, Vec<u8>, Vec<u8>) {
        let record = self.log.lookup(offset).await.unwrap();
        // TODO(AD) Remove clone
        (
            offset + record.len(),
            record.key().clone(),
            record.val().clone(),
        )
    }

    pub async fn put(&mut self, key: Vec<u8>, val: Vec<u8>) -> u64 {
        let record = Record::new(key, val);
        self.log.append(record).await.unwrap()
    }
}

#[cfg(test)]
mod tests {
    // TODO Move log into this module - so logs tests into here.

    use rand::Rng;
    use tempdir::TempDir;

    use super::*;

    #[tokio::test]
    async fn open_new() {
        let dir = TempDir::new("wombatlog").unwrap();
        let not_exist_path = dir.path().join("does_not_exist");

        for n in 0..5 {
            let mut partition = Partition::open(&not_exist_path, "mytopic", n).await;
            put_and_get(&mut partition).await;
        }
    }

    #[tokio::test]
    async fn open_existing() {
        let dir = TempDir::new("wombatlog").unwrap();
        let not_exist_path = dir.path().join("does_not_exist");

        for n in 0..5 {
            let mut partition = Partition::open(&not_exist_path, "mytopic", n).await;

            let mut written = put(&mut partition).await;

            // Recreate the partition and load.
            let mut partition = Partition::open(&not_exist_path, "mytopic", n).await;
            get(&mut partition, &mut written).await;
        }
    }

    // TODO Test expire old segments
    // TODO Test corrupt log

    async fn put_and_get(partition: &mut Partition) {
        let mut written = put(partition).await;

        get(partition, &mut written).await;
    }

    async fn put(partition: &mut Partition) -> Vec<(Vec<u8>, Vec<u8>)> {
        let mut rng = rand::thread_rng();

        let mut written = Vec::<(Vec<u8>, Vec<u8>)>::new();
        for _ in 0..100 {
            let key: Vec<u8> = (0..rng.gen_range(0, 0xff))
                .map(|_| rng.gen_range(0, 0xff))
                .collect();
            let val: Vec<u8> = (0..rng.gen_range(0, 0xff))
                .map(|_| rng.gen_range(0, 0xff))
                .collect();

            let _offset = partition.put(key.clone(), val.clone()).await;
            written.push((key, val));
        }

        written
    }

    async fn get(partition: &mut Partition, written: &Vec<(Vec<u8>, Vec<u8>)>) {
        let mut offset: u64 = 0;
        for (key, val) in written.iter() {
            let (next_offset, key_read, val_read) = partition.get(offset).await;
            assert_eq!(key.clone(), key_read);
            assert_eq!(val.clone(), val_read);
            offset = next_offset;
        }
    }
}
