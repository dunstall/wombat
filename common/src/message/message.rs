use std::marker::Unpin;
use std::vec::Vec;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::message::result::MessageResult;
use crate::message::types::Type;

#[derive(Debug, std::cmp::PartialEq)]
pub struct Message {
    kind: Type,
    payload_len: u64,
    payload: Vec<u8>,
}

impl Message {
    pub fn new(kind: Type, payload: Vec<u8>) -> Message {
        Message {
            kind,
            payload_len: payload.len() as u64,
            payload,
        }
    }

    pub async fn read_from(reader: &mut (impl AsyncRead + Unpin)) -> MessageResult<Message> {
        let kind = Type::decode(reader.read_u16().await?)?;
        let payload_len = reader.read_u64().await?;

        let mut payload = Vec::new();
        payload.resize(payload_len as usize, 0);
        reader.read_exact(payload.as_mut_slice()).await?;

        Ok(Message {
            kind, payload_len, payload,
        })
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
        writer.write_u16(self.kind.encode()).await?;
        writer.write_u64(self.payload_len).await?;
        writer.write_all(&self.payload).await?;
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use std::io::Cursor;

    use super::*;

    #[tokio::test]
    async fn write_to() {
        let mut buf = Cursor::new(vec![0; 100]);

        let msg = Message::new(Type::Consume, vec![1, 2, 3, 4]);
        msg.write_to(&mut buf).await.unwrap();

        assert_eq!(
            &buf.get_ref()[0..14],
            &[0, 1, 0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4]
        );
    }

    #[tokio::test]
    async fn read_from() {
        let mut buf = Cursor::new(vec![0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 2, 1]);
        let msg = Message::read_from(&mut buf).await.unwrap();
        assert_eq!(
            Message::new(Type::Produce, vec![3, 2, 1]),
            msg,
        );
    }
}
