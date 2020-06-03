// mod inmemorylog;
mod log;
mod offsetstore;
mod result;
mod segment;
mod systemsegment;

// TODO Replace uses of system segment with in memory
// TODO Check coverage and rewrite/cleanup tests
// TODO Refactor and doc all public

// pub use inmemorylog::InMemoryLog; TODO inmemorysegment
pub use log::Log;
pub use result::{LogError, LogResult};
pub use segment::Segment;
pub use systemsegment::SystemSegment;
