extern crate byteorder;
extern crate tokio;

mod broker;
mod connection;
mod log;

pub async fn run() {
    println!("Running Wombat broker");

    let b = broker::Broker::new();
    b.listen("0.0.0.0:3110").await;
}
