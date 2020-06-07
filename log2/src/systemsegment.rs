use async_trait::async_trait;
use std::io;
use std::io::SeekFrom;
use std::path::Path;
use tokio::fs;
use tokio::fs::{File, OpenOptions};
use tokio::io::{AsyncReadExt, AsyncWriteExt};

use crate::result::{LogError, LogResult};
use crate::segment;
use crate::Segment;

/// Not thread safe. For concurrent access create multiple segments to the
/// same path/id.
pub struct SystemSegment {
    file: File,
}

#[async_trait]
impl Segment for SystemSegment {
    async fn open(id: u64, dir: &Path) -> LogResult<Box<Self>> {
        fs::create_dir_all(dir).await?;
        let path = dir.join(segment::id_to_name(id));
        let file = OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .append(true)
            .open(path)
            .await?;
        Ok(Box::new(SystemSegment { file }))
    }

    async fn append(&mut self, data: &Vec<u8>) -> LogResult<()> {
        self.file.write_all(data).await?;
        Ok(())
    }

    async fn lookup(&mut self, offset: u64, size: u64) -> LogResult<Vec<u8>> {
        let mut buf = Vec::new();
        buf.resize(size as usize, 0);
        self.file.seek(SeekFrom::Start(offset as u64)).await?;

        if let Err(err) = self.file.read_exact(&mut buf[..]).await {
            if err.kind() == io::ErrorKind::UnexpectedEof {
                Err(LogError::Eof)
            } else {
                Err(LogError::IoError(err))
            }
        } else {
            Ok(buf)
        }
    }

    /// Returns the size of the segment.
    async fn size(&mut self) -> LogResult<u64> {
        Ok(self.file.seek(SeekFrom::End(0)).await?)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use tempdir::TempDir;

    #[tokio::test]
    async fn open_empty() {
        let dir = tmp_dir();
        let mut seg = SystemSegment::open(0x87fa2, &dir.path()).await.unwrap();
        assert_eq!(0, seg.size().await.unwrap());

        seg.append(&vec![1, 2, 3]).await.unwrap();
        assert_eq!(3, seg.size().await.unwrap());
        assert_eq!(vec![1, 2, 3], seg.lookup(0, 3).await.unwrap());
    }

    #[tokio::test]
    async fn open_dir_not_exists() {
        let dir = tmp_dir();
        let mut seg = SystemSegment::open(0x8abb, &dir.path().join("notexist"))
            .await
            .unwrap();
        assert_eq!(0, seg.size().await.unwrap());

        seg.append(&vec![1, 2, 3]).await.unwrap();
        assert_eq!(3, seg.size().await.unwrap());
        assert_eq!(vec![1, 2, 3], seg.lookup(0, 3).await.unwrap());
    }

    #[tokio::test]
    async fn open_load_existing() {
        let dir = tmp_dir();
        let mut seg = SystemSegment::open(0x3ff, &dir.path()).await.unwrap();
        seg.append(&vec![1, 2, 3]).await.unwrap();

        let mut seg = SystemSegment::open(0x3ff, &dir.path()).await.unwrap();
        assert_eq!(3, seg.size().await.unwrap());
        assert_eq!(vec![1, 2, 3], seg.lookup(0, 3).await.unwrap());
    }

    // Test multiple segments refering to the same data.
    #[tokio::test]
    async fn multi_segments_shared() {
        let dir = tmp_dir();

        let mut seg1 = SystemSegment::open(0xf2aa, &dir.path()).await.unwrap();
        seg1.append(&vec![1, 2, 3]).await.unwrap();

        let mut seg2 = SystemSegment::open(0xf2aa, &dir.path()).await.unwrap();
        assert_eq!(3, seg2.size().await.unwrap());
        assert_eq!(vec![1, 2, 3], seg2.lookup(0, 3).await.unwrap());
        seg2.append(&vec![4, 5, 6]).await.unwrap();

        assert_eq!(6, seg1.size().await.unwrap());
        assert_eq!(vec![4, 5, 6], seg1.lookup(3, 3).await.unwrap());
    }

    fn tmp_dir() -> TempDir {
        TempDir::new("dingolog").unwrap()
    }
}
