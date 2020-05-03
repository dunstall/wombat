mod header;

use crc::{crc32, Hasher32};
use std::marker::Unpin;
use std::vec::Vec;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::log::LogResult;
use header::Header;

#[derive(Debug, std::cmp::PartialEq)]
pub struct Record {
    header: Header,
    key: Vec<u8>,
    val: Vec<u8>,
}

impl Record {
    pub fn new(key: Vec<u8>, val: Vec<u8>) -> Record {
        let mut record = Record {
            header: Header::new(key.len() as u64, val.len() as u64),
            key,
            val,
        };
        record.update_crc();
        record
    }

    pub async fn read_from(mut reader: &mut (impl AsyncRead + Unpin)) -> LogResult<Record> {
        let header = Header::read_from(&mut reader).await?;

        let mut key: Vec<u8> = vec![0; header.key_len() as usize];
        reader.read_exact(key.as_mut_slice()).await?;

        let mut val: Vec<u8> = vec![0; header.val_len() as usize];
        reader.read_exact(val.as_mut_slice()).await?;

        Ok(Record { header, key, val })
    }

    pub fn key(&self) -> &Vec<u8> {
        &self.key
    }

    pub fn val(&self) -> &Vec<u8> {
        &self.val
    }

    pub async fn write_to(&self, writer: &mut (impl AsyncWrite + Unpin)) -> LogResult<()> {
        self.header.write_to(writer).await?;
        writer.write_all(&self.key).await?;
        writer.write_all(&self.val).await?;
        writer.flush().await?;
        Ok(())
    }

    fn update_crc(&mut self) {
        self.header.set_crc(self.calc_crc());
    }

    fn calc_crc(&self) -> u32 {
        let mut digest = crc32::Digest::new_with_initial(crc32::IEEE, self.header.calc_crc());
        digest.write(self.key.as_slice());
        digest.write(self.val.as_slice());
        digest.sum32()
    }
}
