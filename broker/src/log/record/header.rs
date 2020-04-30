use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};
use std::clone::Clone;
use std::cmp::PartialEq;
use std::io::Cursor;

use crate::log::result::Result;

pub const LOG_HEADER_SIZE: usize = 20;

#[derive(Clone, Debug, PartialEq)]
pub struct Header {
    // TODO should not be public
    pub timestamp: i64,
    pub key_size: u32, // TODO 64
    pub val_size: u32, // TODO 64
    pub crc: u32,
}

impl Header {
    pub fn decode(buf: [u8; LOG_HEADER_SIZE]) -> Result<Header> {
        let mut rdr = Cursor::new(buf);
        Ok(Header {
            timestamp: rdr.read_i64::<LittleEndian>()?,
            key_size: rdr.read_u32::<LittleEndian>()?,
            val_size: rdr.read_u32::<LittleEndian>()?,
            crc: rdr.read_u32::<LittleEndian>()?,
        })
    }

    pub fn encode(&self) -> Result<[u8; LOG_HEADER_SIZE]> {
        let mut buf: [u8; LOG_HEADER_SIZE] = [0; LOG_HEADER_SIZE];

        buf[0..8]
            .as_mut()
            .write_i64::<LittleEndian>(self.timestamp)?;
        buf[8..12]
            .as_mut()
            .write_u32::<LittleEndian>(self.key_size)?;
        buf[12..16]
            .as_mut()
            .write_u32::<LittleEndian>(self.val_size)?;
        buf[16..20].as_mut().write_u32::<LittleEndian>(self.crc)?;

        Ok(buf)
    }
}

#[cfg(test)]
mod test {
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
            154, 87, 165, 94, 0, 0, 0, 0, 188, 1, 0, 0, 216, 87, 1, 0, 129, 123, 249, 138,
        ];
        assert_eq!(expected, h.encode().unwrap());
    }

    #[test]
    fn decode() {
        let encoded: [u8; LOG_HEADER_SIZE] = [
            40, 97, 45, 24, 134, 13, 156, 95, 33, 0, 0, 0, 0, 42, 245, 42, 53, 99, 42, 85,
        ];
        let expected = Header {
            timestamp: 6889396399552422184,
            key_size: 33,
            val_size: 720710144,
            crc: 1428841269,
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
