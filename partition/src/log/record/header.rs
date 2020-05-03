use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::log::LogResult;

pub const HEADER_SIZE: u64 = 20;

#[derive(Debug, std::cmp::PartialEq)]
pub struct Header {
    key_len: u64,
    val_len: u64,
    crc: u32,
}

impl Header {
    pub fn new(key_len: u64, val_len: u64) -> Header {
        Header {
            key_len,
            val_len,
            crc: 0,
        }
    }

    pub async fn read_from(reader: &mut (impl AsyncRead + Unpin)) -> LogResult<Header> {
        let key_len = reader.read_u64().await?;
        let val_len = reader.read_u64().await?;
        let crc = reader.read_u32().await?;
        Ok(Header {
            key_len,
            val_len,
            crc,
        })
    }

    pub fn key_len(&self) -> usize {
        self.key_len as usize
    }

    pub fn val_len(&self) -> usize {
        self.val_len as usize
    }

    pub fn set_crc(&mut self, crc: u32) {
        self.crc = crc;
    }

    // Returns the CRC of all fields excluding the CRC.
    pub fn calc_crc(&self) -> u32 {
        // TODO
        0
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> LogResult<()> {
        writer.write_u64(self.key_len).await?;
        writer.write_u64(self.val_len).await?;
        writer.write_u32(self.crc).await?;
        Ok(())
    }
}
