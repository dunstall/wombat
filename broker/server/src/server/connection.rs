use std::path::Path;
use tokio;
use tokio::net::TcpStream;

use wombatcore::{ConsumeRequest, ConsumeResponse, Header, ProduceRecord, Type};
use wombatpartition::Partition;

// TODO(AD) Remove unwrap - errors should close the connection
// TODO(AD) Remove clone - use references and lifetimes

// TODO(AD) COnnection per partition to client so open at start only.
pub struct Connection {
    socket: TcpStream,
}

impl Connection {
    pub fn accept(socket: TcpStream) -> Connection {
        Connection { socket }
    }

    pub async fn listen(&mut self) {
        loop {
            let header = Header::read_from(&mut self.socket).await.unwrap();
            match header.kind() {
                Type::Produce => {
                    self.handle_produce().await;
                }
                Type::Consume => {
                    self.handle_consume().await;
                }
            }
        }
    }

    // TODO(AD) Handlers should be unit testable

    async fn handle_produce(&mut self) {
        let req = ProduceRecord::read_from(&mut self.socket).await.unwrap();
        // TODO(AD) partition path configurable
        let mut partition = Partition::open(Path::new("tmp"), req.topic(), req.partition()).await;
        partition.put(req.key().clone(), req.val().clone()).await; // TODO(AD) remove clone
    }

    async fn handle_consume(&mut self) {
        let req = ConsumeRequest::read_from(&mut self.socket).await.unwrap();
        // TODO(AD) partition path configurable
        let mut partition = Partition::open(Path::new("tmp"), req.topic(), req.partition()).await;
        let (next_offset, key, val) = partition.get(req.offset()).await;

        // TODO(AD) Header should be written as part of response. This is messy.
        Header::new(Type::Consume)
            .write_to(&mut self.socket)
            .await
            .unwrap();
        let resp = ConsumeResponse::new(next_offset, key, val);
        resp.write_to(&mut self.socket).await.unwrap();
    }
}
