extern crate byteorder;
extern crate tokio;

mod broker;
mod connection;
mod log;
pub mod message;

use std::path::Path;

use crate::log::{Log, Record};

pub async fn run() {
    println!("Running Wombat broker");

    let mut log = Log::open(Path::new("segments"), 100000).await.unwrap();
    let record = Record::new(b"TEST".to_vec(), b"TEST".to_vec());

    println!("{}", log.append(record).await.unwrap());

    println!("{:?}", log.lookup(0).await.unwrap());

    let b = broker::Broker::new();
    b.listen("0.0.0.0:3110").await;
}
