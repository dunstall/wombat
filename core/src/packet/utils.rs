use std::marker::Unpin;
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use crate::packet::result::MessageResult;

// Reads a vector of unknown size from reader.
pub async fn read_var(reader: &mut (impl AsyncRead + Unpin)) -> MessageResult<Vec<u8>> {
    let len = reader.read_u64().await?;
    let mut item = vec![0; len as usize];
    reader.read_exact(&mut item).await?;
    Ok(item)
}

// Reads a vector of variable size to writer.
pub async fn write_var(item: &[u8], writer: &mut (impl AsyncWrite + Unpin)) -> MessageResult<()> {
    writer.write_u64(item.len() as u64).await?;
    writer.write_all(item).await?;
    Ok(())
}
