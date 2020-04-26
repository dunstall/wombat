extern crate byteorder;

mod log;

use crate::log::filesegment::FileSegment;
use crate::log::log::Log;
use crate::log::logheader::LogHeader;
use crate::log::store::Store;

fn main() {
    println!("Running Wombat broker");

    let mut store = Store::<FileSegment>::new("segments");

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
