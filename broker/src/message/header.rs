use byteorder::{BigEndian, ReadBytesExt, WriteBytesExt};
use std::cmp::PartialEq;
use std::io::Cursor;

use crate::message::result::Result;
use crate::message::types::Type;

pub const MESSAGE_HEADER_SIZE: usize = 10;

#[derive(Debug, PartialEq)]
pub struct Header {
    kind: Type,
    payload_size: u64,
}

impl Header {
    pub fn new(kind: Type, payload_size: u64) -> Header {
        Header { kind, payload_size }
    }

    pub fn payload_size(&self) -> usize {
        self.payload_size as usize
    }

    pub fn decode(buf: [u8; MESSAGE_HEADER_SIZE]) -> Result<Header> {
        let mut rdr = Cursor::new(buf);
        Ok(Header {
            kind: Type::decode(rdr.read_u16::<BigEndian>()?)?,
            payload_size: rdr.read_u64::<BigEndian>()?,
        })
    }

    pub fn encode(&self) -> Result<[u8; MESSAGE_HEADER_SIZE]> {
        let mut buf: [u8; MESSAGE_HEADER_SIZE] = [0; MESSAGE_HEADER_SIZE];
        buf[0..2]
            .as_mut()
            .write_u16::<BigEndian>(self.kind.encode())?;
        buf[2..10]
            .as_mut()
            .write_u64::<BigEndian>(self.payload_size)?;
        Ok(buf)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn encode() {
        let h = Header::new(Type::Produce, 439284);
        let expected: [u8; MESSAGE_HEADER_SIZE] = [0, 0, 0, 0, 0, 0, 0, 6, 179, 244];
        assert_eq!(expected, h.encode().unwrap());
    }

    #[test]
    fn decode() {
        let buf: [u8; MESSAGE_HEADER_SIZE] = [0, 1, 2, 3, 4, 5, 0, 0, 0, 0];
        let expected = Header::new(Type::Consume, 144964032527335424);
        assert_eq!(expected, Header::decode(buf).unwrap());
    }
}
