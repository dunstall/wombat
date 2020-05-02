use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;
use crate::packet::types::Type;

#[derive(Debug, std::cmp::PartialEq)]
pub struct Header {
    kind: Type,
}

impl Header {
    pub fn new(kind: Type) -> Header {
        Header { kind }
    }

    pub fn kind(&self) -> Type {
        self.kind
    }

    pub async fn read_from(reader: &mut (impl AsyncRead + Unpin)) -> MessageResult<Header> {
        Ok(Header {
            kind: Type::decode(reader.read_u16().await?)?,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        writer.write_u16(self.kind.encode()).await?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::io::Cursor;

    use super::*;

    #[tokio::test]
    async fn write_to() {
        let mut buf = Cursor::new(vec![0; 2]);
        let h = Header::new(Type::Consume);
        h.write_to(&mut buf).await.unwrap();
        assert_eq!(&buf.get_ref()[0..2], &[0, 1]);
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![0, 0]);
        let h = Header::read_from(&mut buf).await.unwrap();
        assert_eq!(Header::new(Type::Produce), h);
    }
}
