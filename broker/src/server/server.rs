use tokio;
use tokio::net::{TcpListener, TcpStream, ToSocketAddrs};

use crate::partition::Partition;
use crate::server::result::ServerResult;
use wombatcommon::{ConsumeRequest, ConsumeResponse, Header, ProduceRequest, Type};

pub struct Server;

impl Server {
    pub fn new() -> Server {
        Server {}
    }

    pub async fn listen<A: ToSocketAddrs>(&self, addr: A) -> ServerResult<()> {
        let mut listener = TcpListener::bind(addr).await?;

        loop {
            let (socket, _addr) = listener.accept().await?;
            tokio::spawn(async move {
                handle(socket).await;
            });
        }
    }
}

async fn handle(mut socket: TcpStream) {
    loop {
        let header = Header::read_from(&mut socket).await.unwrap();
        match header.kind() {
            Type::Produce => {
                let req = ProduceRequest::read_from(&mut socket).await.unwrap();
                let partition_n = Partition::map_to_partition(req.key());
                let mut partition = Partition::open(req.topic(), partition_n).await;
                partition.write(req.key(), req.val());
            }
            Type::Consume => {
                Header::new(Type::Consume)
                    .write_to(&mut socket)
                    .await
                    .unwrap();
                let req = ConsumeRequest::read_from(&mut socket).await.unwrap();
                let mut partition = Partition::open(req.topic(), req.partition()).await;
                let (next_offset, key, val) = partition.read(req.offset()).await;
                let resp = ConsumeResponse::new(next_offset, key, val);
                resp.write_to(&mut socket).await.unwrap();
            }
        }
    }
}
