use std::time;
use tokio::net::TcpStream;
use tokio::time::delay_for;

use wombatcommon::{Message, Type};

#[tokio::main]
async fn main() {
    let msg = Message::new(Type::Produce, vec![0, 1, 2, 3, 4, 5, 6, 7, 8, 9]);

    let mut stream = TcpStream::connect("127.0.0.1:3110").await.unwrap();
    loop {
        msg.write_to(&mut stream).await.unwrap();
        delay_for(time::Duration::from_millis(50)).await;
    }
}
