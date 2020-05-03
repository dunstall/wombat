use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;
use crate::packet::utils;

#[derive(Debug, std::cmp::PartialEq)]
pub struct ConsumeResponse {
    offset: u64,
    next_offset: u64,
    key: Vec<u8>,
    val: Vec<u8>,
}

impl ConsumeResponse {
    pub fn new(offset: u64, next_offset: u64, key: Vec<u8>, val: Vec<u8>) -> ConsumeResponse {
        ConsumeResponse {
            offset,
            next_offset,
            key,
            val,
        }
    }

    pub fn offset(&self) -> u64 {
        self.offset
    }

    pub fn next_offset(&self) -> u64 {
        self.next_offset
    }

    pub fn key(&self) -> &Vec<u8> {
        &self.key
    }

    pub fn val(&self) -> &Vec<u8> {
        &self.val
    }

    pub async fn read_from(
        reader: &mut (impl AsyncRead + Unpin),
    ) -> MessageResult<ConsumeResponse> {
        Ok(ConsumeResponse {
            offset: reader.read_u64().await?,
            next_offset: reader.read_u64().await?,

            key: utils::read_var(reader).await?,
            val: utils::read_var(reader).await?,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        writer.write_u64(self.offset).await?;
        writer.write_u64(self.next_offset as u64).await?;
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
        let mut buf = Cursor::new(vec![0; 40]);
        let h = ConsumeResponse::new(0xfffa, 0xffff, vec![1, 2, 3, 4], vec![1, 2, 3, 4]);
        h.write_to(&mut buf).await.unwrap();
        assert_eq!(
            &buf.get_ref()[0..40],
            vec![
                0, 0, 0, 0, 0, 0, 255, 250, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 4, 1,
                2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4
            ]
            .as_slice()
        );
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![
            0, 0, 0, 0, 0, 0, 255, 250, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 4, 1, 2,
            3, 4, 0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4,
        ]);
        let r = ConsumeResponse::read_from(&mut buf).await.unwrap();
        assert_eq!(
            ConsumeResponse::new(0xfffa, 0xffff, vec![1, 2, 3, 4], vec![1, 2, 3, 4]),
            r
        );
    }
}
