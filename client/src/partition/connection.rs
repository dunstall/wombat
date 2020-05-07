use tokio::net::TcpStream;

use wombatcore::{Header, ProduceRecord, Type};

pub struct Connection {
    socket: TcpStream,
}

// TODO(AD) Batch records.
impl Connection {
    pub async fn new() -> Connection {
        Connection {
            socket: TcpStream::connect("127.0.0.1:3110").await.unwrap(),
        }
    }

    pub async fn send(&mut self, record: ProduceRecord) {
        Header::new(Type::Produce)
            .write_to(&mut self.socket)
            .await
            .unwrap();
        record.write_to(&mut self.socket).await.unwrap();
    }
}
