use std::cmp::PartialEq;
use std::io::Cursor;
use std::mem;
use std::vec::Vec;

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

use crate::storage::segment::{LogError, LogResult};

const LOG_HEADER_SIZE: usize = 28;

#[derive(Debug, PartialEq)]
pub struct LogHeader {
    offset: u64,
    timestamp: i64,
    key_size: u32,
    val_size: u32,
    crc: u32,
}

impl LogHeader {
    pub fn decode(enc: Vec<u8>) -> LogResult<LogHeader> {
        if enc.len() < LOG_HEADER_SIZE {
            return Err(LogError::DecodeError("log header too small"));
        }

        let mut rdr = Cursor::new(enc);
        Ok(LogHeader {
            offset: rdr.read_u64::<LittleEndian>()?,
            timestamp: rdr.read_i64::<LittleEndian>()?,
            key_size: rdr.read_u32::<LittleEndian>()?,
            val_size: rdr.read_u32::<LittleEndian>()?,
            crc: rdr.read_u32::<LittleEndian>()?,
        })
    }

    pub fn encode(&self) -> LogResult<Vec<u8>> {
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
        let h = LogHeader {
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
        let expected = LogHeader {
            offset: 2,
            timestamp: 6889396399552422184,
            key_size: 33,
            val_size: 720710144,
            crc: 1428841269,
        };
        assert_eq!(expected, LogHeader::decode(encoded).unwrap());
    }

    #[test]
    fn encode_and_decode() {
        let original = LogHeader {
            offset: 94208892458249824,
            timestamp: 890347582,
            key_size: 244,
            val_size: 8422,
            crc: 3485728435,
        };
        let decoded = LogHeader::decode(original.encode().unwrap()).unwrap();
        assert_eq!(original, decoded);
    }

    #[test]
    #[should_panic]
    fn decode_too_small() {
        let encoded: Vec<u8> = vec![
            2, 0, 0, 0, 0, 0, 0, 0, 40, 97, 45, 24, 134, 13, 156, 95, 33, 0,
        ];
        LogHeader::decode(encoded).unwrap();
    }
}
