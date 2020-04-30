pub mod header;
pub mod result;
pub mod types;

use std::cmp::PartialEq;

use crate::message::result::Result;

#[derive(Debug, PartialEq)]
pub struct Message {
    header: header::Header,
    // TODO payload
}

impl Message {
    pub fn new(header: header::Header) -> Message {
        Message {
            header: header,
        }
    }

    pub fn decode(enc: Vec<u8>) -> Result<Message> {
        Ok(Message {
            header: header::Header::decode(enc)?,
        })
    }

    pub fn encode(&self) -> Result<Vec<u8>> {
        Ok(self.header.encode()?)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn encode() {
        let m = Message::new(header::Header::new(types::Type::Produce));
        let expected: Vec<u8> = vec![0];
        assert_eq!(expected, m.encode().unwrap());
    }

    #[test]
    fn decode_ok() {
        let encoded: Vec<u8> = vec![1];
        let expected = Message::new(header::Header::new(types::Type::Consume));
        assert_eq!(expected, Message::decode(encoded).unwrap());
    }

    #[test]
    #[should_panic]
    fn decode_error() {
        let encoded: Vec<u8> = vec![];
        Message::decode(encoded).unwrap();
    }
}
