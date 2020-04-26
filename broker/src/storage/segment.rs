use crate::storage::log::Log;

pub trait Segment {
    fn new(path: &str) -> Self;
    fn append(&mut self, log: Log);
    fn lookup(&self, offset: u64) -> Log;
    fn size(&self) -> u64;
}
