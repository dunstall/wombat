pub mod header;
pub mod result;
pub mod types;

use std::cmp::PartialEq;

use crate::message::result::Result;

#[derive(Debug, PartialEq)]
pub struct Message {
    header: header::Header,
    payload: Vec<u8>,
}

impl Message {
    pub fn new(header: header::Header, payload: Vec<u8>) -> Message {
        Message { header, payload }
    }

    pub fn encode(&self) -> Result<Vec<u8>> {
        // TODO poor (use sendto)
        let mut enc = self.header.encode()?.to_vec();
        enc.append(&mut self.payload.clone());
        Ok(enc)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn encode() {
        let payload = vec![1, 2, 3, 4];
        let m = Message::new(header::Header::new(types::Type::Produce, 4), payload);
        let expected = vec![0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4];
        assert_eq!(expected, m.encode().unwrap());
    }
}
