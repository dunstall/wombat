use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;

#[derive(Debug, std::cmp::PartialEq)]
pub struct ProduceRequest {
    topic: String,
    key: Vec<u8>,
    val: Vec<u8>,
}

impl ProduceRequest {
    pub fn new(topic: &str, key: Vec<u8>, val: Vec<u8>) -> ProduceRequest {
        ProduceRequest {
            topic: topic.to_string(),
            key,
            val,
        }
    }

    pub fn topic(&self) -> &String {
        &self.topic
    }

    pub fn key(&self) -> &Vec<u8> {
        &self.key
    }

    pub fn val(&self) -> &Vec<u8> {
        &self.val
    }

    pub async fn read_from(reader: &mut (impl AsyncRead + Unpin)) -> MessageResult<ProduceRequest> {
        Ok(ProduceRequest {
            topic: String::from_utf8(ProduceRequest::read_item(reader).await?)?,
            key: ProduceRequest::read_item(reader).await?,
            val: ProduceRequest::read_item(reader).await?,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        self.write_item(self.topic.as_bytes(), writer).await?;
        self.write_item(&self.key, writer).await?;
        self.write_item(&self.val, writer).await?;
        Ok(())
    }

    async fn read_item(reader: &mut (impl AsyncRead + Unpin)) -> MessageResult<Vec<u8>> {
        let len = reader.read_u64().await?;
        let mut item = vec![0; len as usize];
        reader.read_exact(&mut item).await?;
        Ok(item)
    }

    pub async fn write_item(
        &self,
        item: &[u8],
        writer: &mut (impl AsyncWrite + Unpin),
    ) -> MessageResult<()> {
        writer.write_u64(item.len() as u64).await?;
        writer.write_all(item).await?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::io::Cursor;

    use super::*;

    #[tokio::test]
    async fn write_to() {
        let mut buf = Cursor::new(vec![0; 39]);
        let h = ProduceRequest::new("mytopic", vec![1, 2, 3, 4], vec![5, 6, 7, 8]);
        h.write_to(&mut buf).await.unwrap();
        assert_eq!(
            &buf.get_ref()[0..39],
            vec![
                0, 0, 0, 0, 0, 0, 0, 7, 109, 121, 116, 111, 112, 105, 99, 0, 0, 0, 0, 0, 0, 0, 4,
                1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 7, 8
            ]
            .as_slice()
        );
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![
            0, 0, 0, 0, 0, 0, 0, 7, 109, 121, 116, 111, 112, 105, 99, 0, 0, 0, 0, 0, 0, 0, 4, 1, 2,
            3, 4, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 7, 8,
        ]);
        let r = ProduceRequest::read_from(&mut buf).await.unwrap();
        assert_eq!(
            ProduceRequest::new("mytopic", vec![1, 2, 3, 4], vec![5, 6, 7, 8]),
            r
        );
    }
}
