use std::clone::Clone;
use std::cmp::PartialEq;
use std::io::Cursor;
use std::vec::Vec;

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

use crate::log::result::{Error, Result};

pub const LOG_HEADER_SIZE: usize = 28;

/// Represents the header of a log.
#[derive(Clone, Debug, PartialEq)]
pub struct Header {
    // TODO should not be public
    pub offset: u64,
    pub timestamp: i64,
    pub key_size: u32,  // TODO 64
    pub val_size: u32,  // TODO 64
    pub crc: u32,
}

impl Header {
    /// Decodes the given data.
    ///
    /// # Return value
    ///
    /// Returns the first `LOG_HEADER_SIZE` bytes of the data decoded, or an error if the given
    /// data is too short.
    pub fn decode(enc: Vec<u8>) -> Result<Header> {
        if enc.len() < LOG_HEADER_SIZE {
            return Err(Error::DecodeError("log header too small"));
        }

        let mut rdr = Cursor::new(enc);
        Ok(Header {
            offset: rdr.read_u64::<LittleEndian>()?,
            timestamp: rdr.read_i64::<LittleEndian>()?,
            key_size: rdr.read_u32::<LittleEndian>()?,
            val_size: rdr.read_u32::<LittleEndian>()?,
            crc: rdr.read_u32::<LittleEndian>()?,
        })
    }

    /// Encodes this header.
    pub fn encode(&self) -> Result<Vec<u8>> {
        let mut enc = Vec::<u8>::new();
        enc.resize(LOG_HEADER_SIZE, 0);

        enc[0..8].as_mut().write_u64::<LittleEndian>(self.offset)?;
        enc[8..16]
            .as_mut()
            .write_i64::<LittleEndian>(self.timestamp)?;
        enc[16..20]
            .as_mut()
            .write_u32::<LittleEndian>(self.key_size)?;
        enc[20..24]
            .as_mut()
            .write_u32::<LittleEndian>(self.val_size)?;
        enc[24..28].as_mut().write_u32::<LittleEndian>(self.crc)?;

        Ok(enc)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn encode() {
        let h = Header {
            offset: 9999,
            timestamp: 1587894170,
            key_size: 444,
            val_size: 88024,
            crc: 0x8af97b81,
        };
        let expected: Vec<u8> = vec![
            15, 39, 0, 0, 0, 0, 0, 0, 154, 87, 165, 94, 0, 0, 0, 0, 188, 1, 0, 0, 216, 87, 1, 0,
            129, 123, 249, 138,
        ];
        assert_eq!(expected, h.encode().unwrap());
    }

    #[test]
    fn decode() {
        let encoded: Vec<u8> = vec![
            2, 0, 0, 0, 0, 0, 0, 0, 40, 97, 45, 24, 134, 13, 156, 95, 33, 0, 0, 0, 0, 42, 245, 42,
            53, 99, 42, 85,
        ];
        let expected = Header {
            offset: 2,
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
            offset: 94208892458249824,
            timestamp: 890347582,
            key_size: 244,
            val_size: 8422,
            crc: 3485728435,
        };
        let decoded = Header::decode(original.encode().unwrap()).unwrap();
        assert_eq!(original, decoded);
    }

    #[test]
    #[should_panic]
    fn decode_too_small() {
        let encoded: Vec<u8> = vec![
            2, 0, 0, 0, 0, 0, 0, 0, 40, 97, 45, 24, 134, 13, 156, 95, 33, 0,
        ];
        Header::decode(encoded).unwrap();
    }
}
