use std::io::SeekFrom;
use std::path::Path;
use std::process;

use crate::log::header::{Header, LOG_HEADER_SIZE};
use crate::log::log::Log;
use crate::log::result::Result;
use crate::log::segment::Segment;

use async_trait::async_trait;
use tokio::fs::{create_dir_all, File, OpenOptions};
use tokio::io::{AsyncReadExt, AsyncWriteExt};

pub struct FileSegment {
    file: File,
}

#[async_trait]
impl Segment for FileSegment {
    /// Opens a segment at dir/name.
    ///
    /// If dir does not exist it is created.
    async fn open(dir: &str, name: &str) -> Self {
        create_dir_all(dir).await.unwrap_or_else(|err| {
            // Fatal error so crash.
            eprintln!("error opening segment dir: {}", err);
            process::exit(1);
        });
        let file = OpenOptions::new()
            .append(true)
            .create(true)
            .read(true)
            .open(Path::new(dir).join(name))
            .await
            .unwrap_or_else(|err| {
                // Fatal error so crash.
                eprintln!("error opening segment: {}", err);
                process::exit(1);
            });
        FileSegment { file: file }
    }

    /// Appends the given log to the segment and returns the offset.
    async fn append(&mut self, log: Log) -> Result<u64> {
        // Get the current file position as the offset.
        let offset = self.file.seek(SeekFrom::Current(0)).await?;
        self.file.write_all(&log.encode()?).await?;
        Ok(offset)
    }

    /// Looks up the log at the given offset.
    ///
    /// Verifies the log CRC for corrupted data.
    async fn lookup(&mut self, offset: u64) -> Result<Log> {
        self.file.seek(SeekFrom::Start(offset)).await?;
        let mut buffer: [u8; LOG_HEADER_SIZE] = [0; LOG_HEADER_SIZE];
        self.file.read_exact(&mut buffer).await?;

        let header = Header::decode(buffer.to_vec())?;

        let mut key = Vec::new();
        key.resize(header.key_size as usize, 0);
        self.file.read_exact(key.as_mut_slice()).await?;

        let mut val = Vec::new();
        val.resize(header.val_size as usize, 0);
        self.file.read_exact(val.as_mut_slice()).await?;

        let log = Log::new(header, key, val);
        log.verify_crc()?;

        Ok(log)
    }

    async fn size(&mut self) -> Result<u64> {
        return Ok(self.file.seek(SeekFrom::End(0)).await?);
    }
}
