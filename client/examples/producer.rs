// use std::net::SocketAddr;

use wombat::producer::Producer;

fn main() {
    println!("producer");
    let mut p = Producer::new("ws://127.0.0.1:3110").unwrap();
    p.send(b"data etc").unwrap();
}
