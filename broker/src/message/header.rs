use std::cmp::PartialEq;

use crate::message::result::{Error, Result};
use crate::message::types::Type;

pub const MESSAGE_HEADER_SIZE: usize = 1;

#[derive(Debug, PartialEq)]
pub struct Header {
    kind: Type,
}

impl Header {
    pub fn new(kind: Type) -> Header {
        Header { kind }
    }

    pub fn decode(enc: Vec<u8>) -> Result<Header> {
        if enc.len() < MESSAGE_HEADER_SIZE {
            return Err(Error::DecodeError("message header too small"));
        }

        Ok(Header {
            kind: Type::decode(enc[0])?,
        })
    }

    pub fn encode(&self) -> Result<Vec<u8>> {
        let mut enc = Vec::<u8>::new();
        enc.resize(MESSAGE_HEADER_SIZE, 0);
        enc[0] = self.kind.encode();
        Ok(enc)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn encode() {
        let h = Header::new(Type::Produce);
        let expected: Vec<u8> = vec![0];
        assert_eq!(expected, h.encode().unwrap());
    }

    #[test]
    fn decode() {
        let encoded: Vec<u8> = vec![1];
        let expected = Header::new(Type::Consume);
        assert_eq!(expected, Header::decode(encoded).unwrap());
    }

    #[test]
    #[should_panic]
    fn decode_too_small() {
        let encoded: Vec<u8> = vec![];
        Header::decode(encoded).unwrap();
    }
}
