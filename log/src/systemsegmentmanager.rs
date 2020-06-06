use std::fs;
use std::fs::{File, OpenOptions};
use std::path::{Path, PathBuf};

use crate::result::{LogError, LogResult};
use crate::segment;
use crate::segment::Segment;
use crate::segmentmanager::SegmentManager;
use crate::systemsegment::SystemSegment;

pub struct SystemSegmentManager {
    path: PathBuf,
}

impl SystemSegmentManager {
    pub fn new(path: &Path) -> LogResult<SystemSegmentManager> {
        fs::create_dir_all(path)?;
        Ok(SystemSegmentManager {
            path: path.to_path_buf(),
        })
    }

    fn path(&self, id: u64) -> PathBuf {
        self.path.join(segment::id_to_name(id))
    }

    fn path_name(&self, name: String) -> PathBuf {
        self.path.join(name)
    }

    fn open_file(&self, path: PathBuf) -> LogResult<File> {
        Ok(OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .append(true)
            .open(path)?)
    }
}

impl SegmentManager for SystemSegmentManager {
    fn open(&mut self, id: u64) -> LogResult<Box<dyn Segment>> {
        Ok(Box::new(SystemSegment::new(self.open_file(self.path(id))?)))
    }

    fn open_name(&mut self, name: String) -> LogResult<Box<dyn Segment>> {
        Ok(Box::new(SystemSegment::new(
            self.open_file(self.path_name(name))?,
        )))
    }

    fn remove(&mut self, id: u64) -> LogResult<()> {
        match fs::remove_file(&self.path(id)) {
            Ok(()) => Ok(()),
            Err(err) => {
                if err.kind() == std::io::ErrorKind::NotFound {
                    Ok(())
                } else {
                    Err(LogError::IoError(err))
                }
            }
        }
    }

    fn segments(&self) -> LogResult<Vec<u64>> {
        // TODO(AD) iterator
        let mut ids = Vec::new();
        for entry in fs::read_dir(self.path.as_path())? {
            let entry = entry?;
            let name = entry.file_name().into_string()?;
            if let Some(id) = segment::name_to_id(&name) {
                ids.push(id);
            }
        }
        Ok(ids)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use tempdir::TempDir;

    #[test]
    fn new_dir_not_exists() {
        SystemSegmentManager::new(&tmp_dir().path().join("notexist")).unwrap();
    }

    #[test]
    fn new_dir_exists() {
        SystemSegmentManager::new(&tmp_dir().path()).unwrap();
    }

    #[test]
    fn open_segment_not_exists() {
        let dir = tmp_dir();
        let mut manager = SystemSegmentManager::new(&dir.path()).unwrap();
        manager.open(0xaabb).unwrap();
    }

    #[test]
    fn open_segment_exists() {
        let dir = tmp_dir();
        let mut manager = SystemSegmentManager::new(&dir.path()).unwrap();
        manager
            .open(0xaabb)
            .unwrap()
            .append(&vec![1, 2, 3])
            .unwrap();
        // Re-open existing
        assert_eq!(
            vec![1, 2, 3],
            manager.open(0xaabb).unwrap().lookup(3, 0).unwrap()
        );
    }

    #[test]
    fn remove_segment_not_exists() {
        let dir = tmp_dir();
        let mut manager = SystemSegmentManager::new(&dir.path()).unwrap();
        manager.remove(0xaabb).unwrap();
    }

    #[test]
    fn remove_segment_exists() {
        let dir = tmp_dir();
        let mut manager = SystemSegmentManager::new(&dir.path()).unwrap();
        let mut seg = manager.open(0xaabb).unwrap();
        seg.append(&vec![1, 2, 3]).unwrap();
        manager.remove(0xaabb).unwrap();

        // Re-open the segment which should be empty.
        let mut seg = manager.open(0xaabb).unwrap();
        if let Err(LogError::Eof) = seg.lookup(3, 0) {
        } else {
            panic!("expected EOF");
        }
    }

    #[test]
    fn segments_not_empty() {
        let dir = tmp_dir();
        let mut manager = SystemSegmentManager::new(&dir.path()).unwrap();
        manager.open(0xaabb).unwrap();
        manager.open(0xccdd).unwrap();
        assert_eq!(vec![0xccdd, 0xaabb], manager.segments().unwrap());
    }

    #[test]
    fn segments_loaded() {
        let dir = tmp_dir();
        let mut manager = SystemSegmentManager::new(&dir.path()).unwrap();
        manager.open(0xaabb).unwrap();
        manager.open(0xccdd).unwrap();

        // New manager.
        let manager = SystemSegmentManager::new(&dir.path()).unwrap();
        assert_eq!(vec![0xccdd, 0xaabb], manager.segments().unwrap());
    }

    fn tmp_dir() -> TempDir {
        TempDir::new("dingolog").unwrap()
    }
}
