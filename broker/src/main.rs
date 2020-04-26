extern crate byteorder;

mod storage;

use crate::storage::filesegment::FileSegment;
use crate::storage::log::Log;
use crate::storage::logheader::LogHeader;
use crate::storage::logstore::LogStore;

fn main() {
    println!("Running Wombat broker");

    let mut store = LogStore::<FileSegment>::new("segments");

    let header = LogHeader {
        offset: 0,
        timestamp: 0,
        key_size: 4,
        val_size: 4,
        crc: 0,
    };
    let log = Log {
        header,
        key: b"TEST".to_vec(),
        val: b"TEST".to_vec(),
    };

    println!("{}", store.append(log).unwrap());

    println!("{:?}", store.lookup(0).unwrap());
}
