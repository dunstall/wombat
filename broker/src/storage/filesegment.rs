use std::fs;
use std::fs::{File, OpenOptions};
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::Path;
use std::process;

use crate::storage::log::Log;
use crate::storage::logheader::{LogHeader, LOG_HEADER_SIZE};
use crate::storage::segment::{LogResult, Segment};

pub struct FileSegment {
    file: File,
}

impl Segment for FileSegment {
    /// Opens a segment at dir/name.
    ///
    /// If dir does not exist it is created.
    fn open(dir: &str, name: &str) -> Self {
        fs::create_dir_all(dir).unwrap_or_else(|err| {
            // Fatal error so crash.
            eprintln!("error opening segment dir: {}", err);
            process::exit(1);
        });
        let file = OpenOptions::new()
            .append(true)
            .create(true)
            .open(Path::new(dir).join(name))
            .unwrap_or_else(|err| {
                // Fatal error so crash.
                eprintln!("error opening segment: {}", err);
                process::exit(1);
            });
        FileSegment { file: file }
    }

    /// Appends the given log to the segment and returns the offset.
    fn append(&mut self, mut log: Log) -> LogResult<u64> {
        // Get the current file position as the offset.
        let offset = self.file.seek(SeekFrom::Current(0))?;
        self.file.write_all(&log.encode()?)?;
        Ok(offset)
    }

    fn lookup(&mut self, offset: u64) -> LogResult<Log> {
        // TODO can this be moved up to segment? just provide lower level stuff here
        // eg can test with in memory reader/writer

        self.file.seek(SeekFrom::Start(offset))?;
        let mut buffer = [0; LOG_HEADER_SIZE];
        self.file.read_exact(&mut buffer)?;

        let header = LogHeader::decode(buffer.to_vec())?;

        let mut key = Vec::new();
        key.resize(header.key_size as usize, 0);
        self.file.read_exact(key.as_mut_slice())?;

        let mut val = Vec::new();
        key.resize(header.val_size as usize, 0);
        self.file.read_exact(val.as_mut_slice())?;

        let log = Log::new(header, key, val);
        log.verify_crc()?;

        Ok(log)
    }

    fn size(&mut self) -> LogResult<u64> {
        return Ok(self.file.seek(SeekFrom::End(0))?);
    }
}
