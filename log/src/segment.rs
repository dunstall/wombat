use regex::Regex;
use std::path::Path;
use std::time::SystemTime;
use std::vec::Vec;

use crate::result::LogResult;

pub trait Segment {
    fn open(path: &Path) -> LogResult<Box<Self>>;

    fn append(&mut self, data: &Vec<u8>) -> LogResult<u64>;

    fn lookup(&mut self, size: u64, offset: u64) -> LogResult<Vec<u8>>;

    fn modified(&self) -> LogResult<SystemTime>;

    fn remove(&self) -> LogResult<()>;

    fn read_dir(path: &Path) -> LogResult<Vec<u64>>;
}

pub fn id_to_name(id: u64) -> String {
    format!("segment-{:0>20}", id.to_string())
}

pub fn name_to_id(name: &String) -> Option<u64> {
    let re = Regex::new(r"^segment-(\d{20})$").unwrap();
    if let Some(caps) = re.captures(name) {
        if let Some(n) = caps.get(1) {
            // Unwrap as we already checked this is an integer in the regex.
            Some(n.as_str().parse::<u64>().unwrap())
        } else {
            None
        }
    } else {
        None
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn id_to_name_ok() {
        assert_eq!("segment-00000000000000000000", id_to_name(0));
        assert_eq!("segment-00000000000000078642", id_to_name(78642));
        assert_eq!(
            "segment-18446744073709551615",
            id_to_name(0xffffffffffffffff)
        );
    }

    #[test]
    fn name_to_id_ok() {
        assert_eq!(
            Some(0),
            name_to_id(&"segment-00000000000000000000".to_string())
        );
        assert_eq!(
            Some(78642),
            name_to_id(&"segment-00000000000000078642".to_string())
        );
        assert_eq!(
            Some(0xffffffffffffffff),
            name_to_id(&"segment-18446744073709551615".to_string())
        );
    }

    #[test]
    fn name_to_id_none() {
        assert_eq!(None, name_to_id(&"segment-badid".to_string()));
    }
}
