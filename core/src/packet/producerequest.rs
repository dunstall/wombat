use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncWrite};

use crate::packet::result::MessageResult;
use crate::packet::utils;

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
            topic: String::from_utf8(utils::read_var(reader).await?)?,
            key: utils::read_var(reader).await?,
            val: utils::read_var(reader).await?,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        utils::write_var(self.topic.as_bytes(), writer).await?;
        utils::write_var(&self.key, writer).await?;
        utils::write_var(&self.val, writer).await?;
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
