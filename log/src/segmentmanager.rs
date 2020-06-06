use crate::result::LogResult;
use crate::segment::Segment;

pub trait SegmentManager {
    fn open(&mut self, id: u64) -> LogResult<Box<dyn Segment>>;

    fn open_name(&mut self, name: String) -> LogResult<Box<dyn Segment>>;

    fn remove(&mut self, id: u64) -> LogResult<()>;

    fn segments(&self) -> LogResult<Vec<u64>>;
}
