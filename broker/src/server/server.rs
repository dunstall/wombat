use tokio;
use tokio::net::{TcpListener, TcpStream, ToSocketAddrs};

use crate::server::connection::Connection;
use crate::server::result::ServerResult;

pub struct Server;

impl Server {
    pub fn new() -> Server {
        Server {}
    }

    pub async fn listen<A: ToSocketAddrs>(&self, addr: A) -> ServerResult<()> {
        let mut listener = TcpListener::bind(addr).await?;

        loop {
            let (socket, _addr) = listener.accept().await?;
            let mut conn = Connection::accept(socket);
            tokio::spawn(async move {
                conn.listen().await;
            });
        }
    }
}
