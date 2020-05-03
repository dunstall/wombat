use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;

#[derive(Debug, std::cmp::PartialEq)]
pub struct ConsumeResponse {
    offset: u64,
    next_offset: u64,
    payload: Vec<u8>,
}

impl ConsumeResponse {
    pub fn new(offset: u64, next_offset: u64, payload: Vec<u8>) -> ConsumeResponse {
        ConsumeResponse {
            offset,
            next_offset,
            payload,
        }
    }

    pub fn offset(&self) -> u64 {
        self.offset
    }

    pub fn next_offset(&self) -> u64 {
        self.next_offset
    }

    pub fn payload(&self) -> &Vec<u8> {
        &self.payload
    }

    pub async fn read_from(
        reader: &mut (impl AsyncRead + Unpin),
    ) -> MessageResult<ConsumeResponse> {
        let payload_len = reader.read_u64().await?;
        let offset = reader.read_u64().await?;
        let next_offset = reader.read_u64().await?;
        let mut payload = vec![0; payload_len as usize];
        reader.read_exact(&mut payload).await?;
        Ok(ConsumeResponse {
            offset,
            next_offset,
            payload,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        writer.write_u64(self.payload.len() as u64).await?;
        writer.write_u64(self.offset).await?;
        writer.write_u64(self.next_offset as u64).await?;
        writer.write_all(&self.payload).await?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::io::Cursor;

    use super::*;

    #[tokio::test]
    async fn write_to() {
        let mut buf = Cursor::new(vec![0; 28]);
        let h = ConsumeResponse::new(0xfffa, 0xffff, vec![1, 2, 3, 4]);
        h.write_to(&mut buf).await.unwrap();
        assert_eq!(
            &buf.get_ref()[0..28],
            &[
                0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0xff, 0xfa, 0, 0, 0, 0, 0, 0, 0xff, 0xff,
                1, 2, 3, 4
            ]
        );
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![
            0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0xaa, 0x12, 0, 0, 0, 0, 0, 0, 0xab, 0x4, 1,
            2, 3, 4,
        ]);
        let r = ConsumeResponse::read_from(&mut buf).await.unwrap();
        assert_eq!(ConsumeResponse::new(0xaa12, 0xab04, vec![1, 2, 3, 4]), r);
    }
}
