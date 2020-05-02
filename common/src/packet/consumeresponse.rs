use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;

#[derive(Debug, std::cmp::PartialEq)]
pub struct ConsumeResponse {
    payload: Vec<u8>,
}

impl ConsumeResponse {
    pub fn new(payload: Vec<u8>) -> ConsumeResponse {
        ConsumeResponse { payload: payload }
    }

    pub fn payload(&self) -> &Vec<u8> {
        &self.payload
    }

    pub async fn read_from(
        reader: &mut (impl AsyncRead + Unpin),
    ) -> MessageResult<ConsumeResponse> {
        let payload_len = reader.read_u64().await?;
        let mut payload = vec![0; payload_len as usize];
        reader.read_exact(&mut payload).await?;
        Ok(ConsumeResponse { payload: payload })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        writer.write_u64(self.payload.len() as u64).await?;
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
        let mut buf = Cursor::new(vec![0; 12]);
        let h = ConsumeResponse::new(vec![1, 2, 3, 4]);
        h.write_to(&mut buf).await.unwrap();
        assert_eq!(&buf.get_ref()[0..12], &[0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4]);
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4]);
        let r = ConsumeResponse::read_from(&mut buf).await.unwrap();
        assert_eq!(ConsumeResponse::new(vec![1, 2, 3, 4]), r);
    }
}
