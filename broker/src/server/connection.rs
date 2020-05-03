use tokio;
use tokio::net::{TcpListener, TcpStream, ToSocketAddrs};

use crate::partition::Partition;
use crate::server::result::ServerResult;
use wombatcommon::{ConsumeRequest, ConsumeResponse, Header, ProduceRequest, Type};

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

    async fn handle_produce(&mut self) {
        let req = ProduceRequest::read_from(&mut self.socket).await.unwrap();
        let partition_n = Partition::map_to_partition(req.key());
        let mut partition = Partition::open(req.topic(), partition_n).await;
        partition.write(req.key(), req.val());
    }

    async fn handle_consume(&mut self) {
        let req = ConsumeRequest::read_from(&mut self.socket).await.unwrap();
        let mut partition = Partition::open(req.topic(), req.partition()).await;
        let (next_offset, key, val) = partition.read(req.offset()).await;
        let resp = ConsumeResponse::new(next_offset, key, val);
        resp.write_to(&mut self.socket).await.unwrap();
    }
}
