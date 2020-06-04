mod inmemorysegment;
mod log;
mod offsetstore;
mod result;
mod segment;
mod systemsegment;

// TODO Replace uses of system segment with in memory
// TODO Check coverage and rewrite/cleanup tests
// TODO Refactor and doc all public

pub use inmemorysegment::InMemorySegment;
pub use log::Log;
pub use result::{LogError, LogResult};
pub use segment::Segment;
pub use systemsegment::SystemSegment;
