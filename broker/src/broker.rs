use tokio;
use tokio::net::{TcpListener, ToSocketAddrs};

use crate::connection::Connection;

pub struct Broker;

impl Broker {
    pub fn new() -> Broker {
        Broker {}
    }

    pub async fn listen<A: ToSocketAddrs>(&self, addr: A) {
        println!("listen");

        let mut listener = TcpListener::bind(addr).await.unwrap();

        loop {
            let (socket, _addr) = listener.accept().await.unwrap();
            tokio::spawn(async move {
                let mut conn = Connection::new(socket);
                conn.handle().await;
            });
        }
    }
}
