use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;
use crate::packet::utils;

#[derive(Debug, std::cmp::PartialEq)]
pub struct ProduceRecord {
    topic: String,
    partition: u32,
    key: Vec<u8>,
    val: Vec<u8>,
}

impl ProduceRecord {
    pub fn new(topic: &str, key: Vec<u8>, val: Vec<u8>) -> ProduceRecord {
        ProduceRecord {
            topic: topic.to_string(),
            partition: 0,
            key,
            val,
        }
    }

    pub fn topic(&self) -> &String {
        &self.topic
    }

    pub fn partition(&self) -> u32 {
        self.partition
    }

    pub fn key(&self) -> &Vec<u8> {
        &self.key
    }

    pub fn val(&self) -> &Vec<u8> {
        &self.val
    }

    pub fn set_partition(&mut self, partition: u32) {
        self.partition = partition;
    }

    pub async fn read_from(reader: &mut (impl AsyncRead + Unpin)) -> MessageResult<ProduceRecord> {
        Ok(ProduceRecord {
            topic: String::from_utf8(utils::read_var(reader).await?)?,
            partition: reader.read_u32().await?,
            key: utils::read_var(reader).await?,
            val: utils::read_var(reader).await?,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        utils::write_var(self.topic.as_bytes(), writer).await?;
        writer.write_u32(self.partition).await?;
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
        let mut buf = Cursor::new(vec![0; 43]);
        let mut r = ProduceRecord::new("mytopic", vec![1, 2, 3, 4], vec![5, 6, 7, 8]);
        r.set_partition(0xffaa);
        r.write_to(&mut buf).await.unwrap();
        assert_eq!(
            &buf.get_ref()[0..43],
            vec![
                0, 0, 0, 0, 0, 0, 0, 7, 109, 121, 116, 111, 112, 105, 99, 0, 0, 0xff, 0xaa, 0, 0,
                0, 0, 0, 0, 0, 4, 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 7, 8
            ]
            .as_slice()
        );
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![
            0, 0, 0, 0, 0, 0, 0, 7, 109, 121, 116, 111, 112, 105, 99, 0, 0, 0xff, 0xaa, 0, 0, 0, 0,
            0, 0, 0, 4, 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 7, 8,
        ]);
        let r = ProduceRecord::read_from(&mut buf).await.unwrap();

        let mut expected = ProduceRecord::new("mytopic", vec![1, 2, 3, 4], vec![5, 6, 7, 8]);
        expected.set_partition(0xffaa);
        assert_eq!(expected, r);
    }
}
