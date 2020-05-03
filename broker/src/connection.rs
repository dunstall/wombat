use std::path::Path;
use tokio::net::TcpStream;

use crate::log::{Log, Record};
use wombatcommon::{ConsumeRequest, ConsumeResponse, Header, ProduceRequest, Type};

pub struct Connection {
    socket: TcpStream,
}

impl Connection {
    pub fn new(socket: TcpStream) -> Connection {
        Connection { socket: socket }
    }

    pub async fn handle(&mut self) {
        // TODO Must be shared and protected
        let mut log = Log::open(Path::new("segments"), 100000).await.unwrap();

        loop {
            let header = Header::read_from(&mut self.socket).await.unwrap();
            match header.kind() {
                Type::Produce => {
                    let req = ProduceRequest::read_from(&mut self.socket).await.unwrap();
                    // TODO remove clone
                    log.append(Record::new(vec![], req.payload().clone()))
                        .await
                        .unwrap();
                }
                Type::Consume => {
                    Header::new(Type::Consume)
                        .write_to(&mut self.socket)
                        .await
                        .unwrap();
                    let req = ConsumeRequest::read_from(&mut self.socket).await.unwrap();
                    let record = log.lookup(req.offset()).await.unwrap();
                    // TODO remove clone
                    let resp = ConsumeResponse::new(
                        req.offset(),
                        req.offset() + 20 + record.key().len() as u64 + record.val().len() as u64,
                        record.val().clone(),
                    );
                    resp.write_to(&mut self.socket).await.unwrap();
                }
            }
        }
    }
}
