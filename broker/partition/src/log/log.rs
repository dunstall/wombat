use std::io::SeekFrom;
use std::path::Path;
use tokio::fs;
use tokio::fs::{File, OpenOptions};

use crate::log::{LogResult, Record};

pub struct Log {
    file: File,
}

// TODO(AD) Multi segment and expiry
impl Log {
    pub async fn open(dir: &Path) -> LogResult<Log> {
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
}
