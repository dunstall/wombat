extern crate byteorder;
extern crate tokio;

mod log;

use crate::log::filesegment::FileSegment;
use crate::log::header::Header;
use crate::log::log::Log;
use crate::log::store::Store;

#[tokio::main]
async fn main() {
    println!("Running Wombat broker");

    let mut store = Store::<FileSegment>::new("segments").await;

    let header = Header {
        timestamp: 0,
        key_size: 4,
        val_size: 4,
        crc: 0,
    };
    let mut log = Log {
        header,
        key: b"TEST".to_vec(),
        val: b"TEST".to_vec(),
    };
    log.update_crc().unwrap();

    println!("{}", store.append(log).await.unwrap());

    println!("{:?}", store.lookup(0).await.unwrap());
}
