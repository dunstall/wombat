use crate::connection::Connection;

use tokio;
use tokio::net::{TcpListener, ToSocketAddrs};

pub struct Broker;

impl Broker {
    pub fn new() -> Broker {
        Broker {}
    }

    pub async fn listen<A: ToSocketAddrs>(&self, addr: A) {
        // TODO Listener trait - then have own TcpListener that returns Connection

        let mut listener = TcpListener::bind(addr).await.unwrap();

        loop {
            let (socket, addr) = listener.accept().await.unwrap();
            tokio::spawn(async move {
                let mut conn = Connection::new(socket, addr);
                conn.handle().await;
            });
        }
    }
}
