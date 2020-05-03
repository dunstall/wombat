use std::io::SeekFrom;
use std::path::Path;
use tokio::fs;
use tokio::fs::{File, OpenOptions};

use crate::partition::log::{LogResult, Record};

// Not thread safe as read and write in multiple chunks so may interleave.
pub struct Log {
    file: File,
}

// TODO(AD) Multi segment and expiry
impl Log {
    pub async fn open(dir: &Path, max_segment_size: u64) -> LogResult<Log> {
        fs::create_dir_all(dir).await?;
        let file = OpenOptions::new()
            .append(true)
            .create(true)
            .read(true)
            .open(dir.join("segment"))
            .await?;
        Ok(Log { file })
    }

    pub async fn append(&mut self, record: Record) -> LogResult<u64> {
        let offset = self.file.seek(SeekFrom::End(0)).await?;
        record.write_to(&mut self.file).await?;
        Ok(offset)
    }

    pub async fn lookup(&mut self, offset: u64) -> LogResult<Record> {
        self.file.seek(SeekFrom::Start(offset)).await?;
        Record::read_from(&mut self.file).await
    }

    // pub async fn expire(since) TODO
}

#[cfg(test)]
mod tests {
    use tempdir::TempDir;

    use super::*;

    #[tokio::test]
    async fn open_dir_not_exist() {
        let dir = TempDir::new("wombatlog").unwrap();
        let not_exist_path = dir.path().join("does_not_exist");
        let mut log = Log::open(&not_exist_path, 0).await.unwrap();

        let record = Record::new(b"testkey".to_vec(), b"testval".to_vec());
        assert_eq!(0, log.append(record).await.unwrap());
        assert_eq!(
            Record::new(b"testkey".to_vec(), b"testval".to_vec()),
            log.lookup(0).await.unwrap()
        );
    }

    #[tokio::test]
    async fn open_dir_empty() {
        let dir = TempDir::new("wombatlog").unwrap();
        let mut log = Log::open(&dir.path(), 0).await.unwrap();

        let record = Record::new(b"testkey".to_vec(), b"testval".to_vec());
        assert_eq!(0, log.append(record).await.unwrap());
        assert_eq!(
            Record::new(b"testkey".to_vec(), b"testval".to_vec()),
            log.lookup(0).await.unwrap()
        );
    }

    #[tokio::test]
    async fn open_existing_log() {
        let dir = TempDir::new("wombatlog").unwrap();
        let mut log = Log::open(&dir.path(), 2000).await.unwrap();

        for offset in (0..1000).step_by(37) {
            // Ensure each record is unique.
            let val = format!("{:0>10}", offset.to_string());
            let record = Record::new(b"testkey".to_vec(), val.as_bytes().to_vec());

            assert_eq!(offset, log.append(record).await.unwrap());
        }

        // Create a new log object and read the values written before.
        let mut log = Log::open(&dir.path(), 2000).await.unwrap();
        for offset in (0..1000).step_by(37) {
            // Ensure each record is unique.
            let val = format!("{:0>10}", offset.to_string());
            let record = Record::new(b"testkey".to_vec(), val.as_bytes().to_vec());

            assert_eq!(record, log.lookup(offset).await.unwrap());
        }
    }

    #[tokio::test]
    async fn lookup_random() {
        let dir = TempDir::new("wombatlog").unwrap();
        let mut log = Log::open(&dir.path(), 2000).await.unwrap();

        for offset in (0..1000).step_by(37) {
            // Ensure each record is unique.
            let val = format!("{:0>10}", offset.to_string());
            let record = Record::new(b"testkey".to_vec(), val.as_bytes().to_vec());

            assert_eq!(offset, log.append(record).await.unwrap());
        }

        let offset = 370;
        let val = format!("{:0>10}", offset.to_string());
        let record = Record::new(b"testkey".to_vec(), val.as_bytes().to_vec());

        assert_eq!(record, log.lookup(offset).await.unwrap());
    }

    // TODO Multi segment

    // TODO Expire old segment

    // TODO Corrupt log
}
