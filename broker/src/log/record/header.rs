use byteorder::{BigEndian, ReadBytesExt, WriteBytesExt};
use std::clone::Clone;
use std::cmp::PartialEq;
use std::io::Cursor;

use crate::log::result::Result;

// TODO RECORD_HEADER_SIZE
pub const LOG_HEADER_SIZE: usize = 28;

#[derive(Clone, Debug, PartialEq)]
pub struct Header {
    // TODO should not be public
    pub timestamp: i64,
    pub key_size: u64,
    pub val_size: u64,
    pub crc: u32,
}

impl Header {
    // TODO new (then make members private)

    pub fn decode(buf: [u8; LOG_HEADER_SIZE]) -> Result<Header> {
        let mut rdr = Cursor::new(buf);
        Ok(Header {
            timestamp: rdr.read_i64::<BigEndian>()?,
            key_size: rdr.read_u64::<BigEndian>()?,
            val_size: rdr.read_u64::<BigEndian>()?,
            crc: rdr.read_u32::<BigEndian>()?,
        })
    }

    pub fn encode(&self) -> Result<[u8; LOG_HEADER_SIZE]> {
        let mut buf: [u8; LOG_HEADER_SIZE] = [0; LOG_HEADER_SIZE];
        buf[0..8].as_mut().write_i64::<BigEndian>(self.timestamp)?;
        buf[8..16].as_mut().write_u64::<BigEndian>(self.key_size)?;
        buf[16..24].as_mut().write_u64::<BigEndian>(self.val_size)?;
        buf[24..28].as_mut().write_u32::<BigEndian>(self.crc)?;
        Ok(buf)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn encode() {
        let h = Header {
            timestamp: 1587894170,
            key_size: 444,
            val_size: 88024,
            crc: 0x8af97b81,
        };
        let expected: [u8; LOG_HEADER_SIZE] = [
            0, 0, 0, 0, 94, 165, 87, 154, 0, 0, 0, 0, 0, 0, 1, 188, 0, 0, 0, 0, 0, 1, 87, 216, 138,
            249, 123, 129,
        ];
        assert_eq!(expected, h.encode().unwrap());
    }

    #[test]
    fn decode() {
        let encoded: [u8; LOG_HEADER_SIZE] = [
            40, 97, 45, 24, 134, 13, 156, 95, 0, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0, 0, 0, 42, 245, 42,
            53, 99, 42, 85,
        ];
        let expected = Header {
            timestamp: 2909656417609555039,
            key_size: 553648128,
            val_size: 2815274,
            crc: 895691349,
        };
        assert_eq!(expected, Header::decode(encoded).unwrap());
    }

    #[test]
    fn encode_and_decode() {
        let original = Header {
            timestamp: 890347582,
            key_size: 244,
            val_size: 8422,
            crc: 3485728435,
        };
        let decoded = Header::decode(original.encode().unwrap()).unwrap();
        assert_eq!(original, decoded);
    }
}
