pub mod header;
pub mod result;
pub mod types;

use std::cmp::PartialEq;

use crate::message::result::Result;

#[derive(Debug, PartialEq)]
pub struct Message {
    header: header::Header,
}

impl Message {
    pub fn new(header: header::Header) -> Message {
        Message { header: header }
    }

    pub fn encode(&self) -> Result<[u8; header::MESSAGE_HEADER_SIZE]> {
        Ok(self.header.encode()?)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn encode() {
        let m = Message::new(header::Header::new(types::Type::Produce, 242));
        let expected: [u8; header::MESSAGE_HEADER_SIZE] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 242];
        assert_eq!(expected, m.encode().unwrap());
    }
}
