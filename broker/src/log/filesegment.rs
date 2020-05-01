use async_trait::async_trait;
use std::io::SeekFrom;
use std::path::Path;
use std::process;
use tokio::fs::{create_dir_all, File, OpenOptions};
use tokio::io::{AsyncReadExt, AsyncWriteExt};

use crate::log::record::header::{Header, LOG_HEADER_SIZE};
use crate::log::record::Record;
use crate::log::result::Result;
use crate::log::segment::{Len, Segment};

pub struct FileSegment {
    file: File,
    size: u64,
    max_size: u64,
}
#[async_trait]
impl Segment for FileSegment {
    /// Opens a segment at dir/name. If dir does not exist it is created.
    async fn open(dir: &Path, name: &str, max_size: u64) -> Self {
        create_dir_all(&dir).await.unwrap_or_else(|err| {
            // Fatal error.
            eprintln!("error opening segment dir: {}", err);
            process::exit(1);
        });
        let mut file = OpenOptions::new()
            .append(true)
            .create(true)
            .read(true)
            .open(Path::new(&dir).join(name))
            .await
            .unwrap_or_else(|err| {
                // Fatal error.
                eprintln!("error opening segment: {}", err);
                process::exit(1);
            });
        let size = file.seek(SeekFrom::End(0)).await.unwrap_or_else(|err| {
            // Fatal error.
            eprintln!("error seeking segment file: {}", err);
            process::exit(1);
        });
        FileSegment {
            file: file,
            size: size,
            max_size: max_size,
        }
    }

    /// Appends the given record to the segment and returns the offset.
    async fn append(&mut self, record: Record) -> Result<u64> {
        let offset = self.size;
        self.file.write_all(&record.encode()?).await?;
        self.size = self.file.seek(SeekFrom::Current(0)).await?;
        Ok(offset)
    }

    /// Looks up the record at the given offset.
    ///
    /// Verifies the record CRC for corrupted data.
    async fn lookup(&mut self, offset: u64) -> Result<Record> {
        // TODO have one buffer allocated once?

        self.file.seek(SeekFrom::Start(offset)).await?;
        let mut buffer: [u8; LOG_HEADER_SIZE] = [0; LOG_HEADER_SIZE];
        self.file.read_exact(&mut buffer).await?;

        let header = Header::decode(buffer)?;

        let mut key = Vec::new();
        key.resize(header.key_size as usize, 0);
        self.file.read_exact(key.as_mut_slice()).await?;

        let mut val = Vec::new();
        val.resize(header.val_size as usize, 0);
        self.file.read_exact(val.as_mut_slice()).await?;

        let record = Record::new(header, key, val);
        record.verify_crc()?;

        Ok(record)
    }
}

impl Len for FileSegment {
    fn len(&self) -> u64 {
        self.size
    }

    fn is_full(&self) -> bool {
        self.len() >= self.max_size
    }
}

#[cfg(test)]
mod tests {
    use tempdir::TempDir;

    use super::*;

    #[tokio::test]
    async fn open_empty_file() {
        let dir = TempDir::new("wombat-log").unwrap();

        let segment = FileSegment::open(&dir.path(), "segment-0", 20).await;
        assert_eq!(true, segment.is_empty());
        assert_eq!(false, segment.is_full());
    }

    #[tokio::test]
    async fn open_non_empty_file() {
        let dir = TempDir::new("wombat-log").unwrap();

        let file_path = dir.path().join("segment-0");
        let mut segment_file = File::create(file_path).await.unwrap();
        segment_file.write_all(b"hello, world").await.unwrap();

        let segment = FileSegment::open(&dir.path(), "segment-0", 20).await;
        assert_eq!(12, segment.len());
        assert_eq!(false, segment.is_full());
    }

    #[tokio::test]
    async fn open_full_file() {
        let dir = TempDir::new("wombat-log").unwrap();

        let file_path = dir.path().join("segment-0");
        let mut segment_file = File::create(file_path).await.unwrap();
        segment_file.write_all(b"hello, world").await.unwrap();

        let segment = FileSegment::open(&dir.path(), "segment-0", 10).await;
        assert_eq!(12, segment.len());
        assert_eq!(true, segment.is_full());
    }

    #[tokio::test]
    async fn append_multiple_records() {
        let dir = TempDir::new("wombat-log").unwrap();

        let mut segment = FileSegment::open(&dir.path(), "segment-0", 1000).await;

        for offset in (0..1000).step_by(50) {
            assert_eq!(offset, segment.append(example_record()).await.unwrap());
            assert_eq!(offset + 50, segment.len());
            assert_eq!(example_record(), segment.lookup(offset).await.unwrap());
        }

        assert_eq!(true, segment.is_full());
    }

    #[tokio::test]
    #[should_panic]
    async fn corrupt_record() {
        let dir = TempDir::new("wombat-log").unwrap();

        let mut segment = FileSegment::open(&dir.path(), "segment-0", 1000).await;

        let mut corrupt_record = example_record();
        // Change timestamp to make CRC invalid.
        corrupt_record.header.timestamp = 9873200;

        segment.append(corrupt_record).await.unwrap();
        segment.lookup(0).await.unwrap();
    }

    fn example_record() -> Record {
        let key = b"test-key".to_vec();
        let val = b"test-value-123".to_vec();
        let h = Header {
            timestamp: 9873248,
            key_size: key.len() as u64,
            val_size: val.len() as u64,
            crc: 0xa7be64b2,
        };
        Record::new(h, key, val)
    }
}
