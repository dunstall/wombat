use std::net::SocketAddr;
use client::consumer::Consumer;

fn main() {
    let c = Consumer::new(SocketAddr::from(([127, 0, 0, 1], 3110)));
    c.poll().unwrap();
}
