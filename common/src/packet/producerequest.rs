use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;

#[derive(Debug, std::cmp::PartialEq)]
pub struct ProduceRequest {
    payload: Vec<u8>,
}

impl ProduceRequest {
    pub fn new(payload: Vec<u8>) -> ProduceRequest {
        ProduceRequest { payload: payload }
    }

    pub fn payload(&self) -> &Vec<u8> {
        &self.payload
    }

    pub async fn read_from(
        reader: &mut (impl AsyncRead + Unpin),
    ) -> MessageResult<ProduceRequest> {
        let payload_len = reader.read_u64().await?;
        let mut payload = vec![0; payload_len as usize];
        reader.read_exact(&mut payload).await?;
        Ok(ProduceRequest { payload: payload })
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
        let h = ProduceRequest::new(vec![1, 2, 3, 4]);
        h.write_to(&mut buf).await.unwrap();
        assert_eq!(&buf.get_ref()[0..12], &[0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4]);
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4]);
        let r = ProduceRequest::read_from(&mut buf).await.unwrap();
        assert_eq!(ProduceRequest::new(vec![1, 2, 3, 4]), r);
    }
}
