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

const MAX_SEGMENT_SIZE: u64 = 1_000_000_000;

pub struct FileSegment {
    file: File,
    size: u64,
}

#[async_trait]
impl Segment for FileSegment {
    /// Opens a segment at dir/name. If dir does not exist it is created.
    async fn open(dir: &str, name: &str) -> Self {
        create_dir_all(dir).await.unwrap_or_else(|err| {
            // Fatal error.
            eprintln!("error opening segment dir: {}", err);
            process::exit(1);
        });
        let mut file = OpenOptions::new()
            .append(true)
            .create(true)
            .read(true)
            .open(Path::new(dir).join(name))
            .await
            .unwrap_or_else(|err| {
                // Fatal error.
                eprintln!("error opening segment: {}", err);
                process::exit(1);
            });
        let size = file.seek(SeekFrom::Current(0)).await.unwrap_or_else(|err| {
            // Fatal error.
            eprintln!("error seeking segment file: {}", err);
            process::exit(1);
        });
        FileSegment {
            file: file,
            size: size,
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
        self.len() >= MAX_SEGMENT_SIZE
    }
}
