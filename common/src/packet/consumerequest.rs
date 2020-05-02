use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;

#[derive(Debug, std::cmp::PartialEq)]
pub struct ConsumeRequest {
    offset: u64,
}

impl ConsumeRequest {
    pub fn new(offset: u64) -> ConsumeRequest {
        ConsumeRequest { offset: offset }
    }

    pub fn offset(&self) -> u64 {
        self.offset
    }

    pub async fn read_from(reader: &mut (impl AsyncRead + Unpin)) -> MessageResult<ConsumeRequest> {
        Ok(ConsumeRequest {
            offset: reader.read_u64().await?,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        writer.write_u64(self.offset).await?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::io::Cursor;

    use super::*;

    #[tokio::test]
    async fn write_to() {
        let mut buf = Cursor::new(vec![0; 8]);

        let h = ConsumeRequest::new(0x1234);
        h.write_to(&mut buf).await.unwrap();

        assert_eq!(&buf.get_ref()[0..8], &[0, 0, 0, 0, 0, 0, 0x12, 0x34]);
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![0, 0, 0xff, 0, 0, 0x1a, 0, 3]);
        let r = ConsumeRequest::read_from(&mut buf).await.unwrap();
        assert_eq!(ConsumeRequest::new(0xff00001a0003), r);
    }
}
