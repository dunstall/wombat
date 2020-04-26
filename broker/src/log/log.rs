use std::vec::Vec;

use crc::{crc32, Hasher32};

use crate::log::header::Header;
use crate::log::result::{Error, Result};

#[derive(Debug)]
pub struct Log {
    pub header: Header,
    pub key: Vec<u8>,
    pub val: Vec<u8>,
}

impl Log {
    pub fn new(header: Header, key: Vec<u8>, val: Vec<u8>) -> Log {
        Log { header, key, val }
    }

    pub fn encode(&self) -> Result<Vec<u8>> {
        let mut enc = self.header.encode()?;
        enc.append(&mut self.key.clone());
        enc.append(&mut self.val.clone());
        Ok(enc)
    }

    pub fn verify_crc(&self) -> Result<()> {
        let mut h = self.header.clone();
        h.crc = 0;

        let mut digest = crc32::Digest::new_with_initial(crc32::IEEE, 0u32);
        digest.write(h.encode()?.as_mut_slice());
        digest.write(self.key.clone().as_mut_slice());
        digest.write(self.val.clone().as_mut_slice());

        if digest.sum32() == self.header.crc {
            Ok(())
        } else {
            Err(Error::LogCorrupted)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn encode() {
        let key = b"test-key".to_vec();
        let val = b"test-value-123".to_vec();
        let h = Header {
            offset: 931,
            timestamp: 9873248,
            key_size: key.len() as u32,
            val_size: val.len() as u32,
            crc: 0x8af97b81,
        };
        let mut expected: Vec<u8> = h.encode().unwrap();
        expected.append(&mut key.clone());
        expected.append(&mut val.clone());

        let log = Log::new(h, key.clone(), val.clone());

        assert_eq!(expected, log.encode().unwrap());
    }

    #[test]
    fn verify_crc_ok() {
        let key = b"test-key".to_vec();
        let val = b"test-value-123".to_vec();
        let h = Header {
            offset: 931,
            timestamp: 9873248,
            key_size: key.len() as u32,
            val_size: val.len() as u32,
            crc: 0x919f271,
        };
        let log = Log::new(h, key, val);
        log.verify_crc().unwrap();
    }

    #[test]
    #[should_panic]
    fn verify_crc_fail() {
        let key = b"test-key".to_vec();
        let val = b"test-value-123".to_vec();
        let h = Header {
            offset: 931,
            timestamp: 9873248,
            key_size: key.len() as u32,
            val_size: val.len() as u32,
            // Bad CRC.
            crc: 0x1234567,
        };
        let log = Log::new(h, key, val);
        log.verify_crc().unwrap();
    }
}
