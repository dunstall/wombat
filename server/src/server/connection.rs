use std::path::Path;
use tokio;
use tokio::net::TcpStream;

use wombatcore::{ConsumeRequest, ConsumeResponse, Header, ProduceRequest, Type};
use wombatpartition::Partition;

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
        let mut partition = Partition::open(Path::new("tmp"), req.topic(), partition_n).await;
        partition.put(req.key().clone(), req.val().clone()).await; // TODO remove clone
    }

    async fn handle_consume(&mut self) {
        Header::new(Type::Consume)
            .write_to(&mut self.socket)
            .await
            .unwrap();

        let req = ConsumeRequest::read_from(&mut self.socket).await.unwrap();
        let mut partition = Partition::open(Path::new("tmp"), req.topic(), req.partition()).await;
        let (next_offset, key, val) = partition.get(req.offset()).await;
        let resp = ConsumeResponse::new(next_offset, key, val);
        resp.write_to(&mut self.socket).await.unwrap();
    }
}
