extern crate byteorder;
extern crate tokio;

mod broker;
mod connection;
mod log;
pub mod message;

use crate::log::filesegment::FileSegment;
use crate::log::header::Header;
use crate::log::record::Record;
use crate::log::store::Store;

pub async fn run() {
    println!("Running Wombat broker");

    let mut store = Store::<FileSegment>::open("segments").await;

    let header = Header {
        timestamp: 0,
        key_size: 4,
        val_size: 4,
        crc: 0,
    };
    let mut record = Record {
        header,
        key: b"TEST".to_vec(),
        val: b"TEST".to_vec(),
    };
    record.update_crc().unwrap();

    println!("{}", store.append(record).await.unwrap());

    println!("{:?}", store.lookup(0).await.unwrap());

    let b = broker::Broker::new();
    b.listen("0.0.0.0:3110").await;
}
