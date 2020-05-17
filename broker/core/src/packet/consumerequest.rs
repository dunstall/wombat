use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;
use crate::packet::utils;

#[derive(Debug, std::cmp::PartialEq)]
pub struct ConsumeRequest {
    topic: String,
    offset: u64,
    partition: u32,
}

impl ConsumeRequest {
    pub fn new(topic: &str, offset: u64, partition: u32) -> ConsumeRequest {
        ConsumeRequest {
            topic: topic.to_string(),
            offset,
            partition,
        }
    }

    pub fn topic(&self) -> &String {
        &self.topic
    }

    pub fn offset(&self) -> u64 {
        self.offset
    }

    pub fn partition(&self) -> u32 {
        self.partition
    }

    pub async fn read_from(reader: &mut (impl AsyncRead + Unpin)) -> MessageResult<ConsumeRequest> {
        Ok(ConsumeRequest {
            topic: String::from_utf8(utils::read_var(reader).await?)?,
            offset: reader.read_u64().await?,
            partition: reader.read_u32().await?,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        utils::write_var(self.topic.as_bytes(), writer).await?;
        writer.write_u64(self.offset).await?;
        writer.write_u32(self.partition).await?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::io::Cursor;

    use super::*;

    #[tokio::test]
    async fn write_to() {
        let mut buf = Cursor::new(vec![0; 27]);

        let h = ConsumeRequest::new("mytopic", 0x1234, 0xff);
        h.write_to(&mut buf).await.unwrap();

        assert_eq!(
            &buf.get_ref()[0..27],
            &[
                0, 0, 0, 0, 0, 0, 0, 7, 109, 121, 116, 111, 112, 105, 99, 0, 0, 0, 0, 0, 0, 0x12,
                0x34, 0, 0, 0, 0xff
            ]
        );
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![
            0, 0, 0, 0, 0, 0, 0, 7, 109, 121, 116, 111, 112, 105, 99, 0, 0, 0xff, 0, 0, 0x1a, 0, 3,
            0xff, 0xa, 0xb, 0xc,
        ]);
        let r = ConsumeRequest::read_from(&mut buf).await.unwrap();
        assert_eq!(
            ConsumeRequest::new("mytopic", 0xff00001a0003, 0xff0a0b0c),
            r
        );
    }
}
