use tokio::net::TcpStream;

use crate::result::WombatResult;
use wombatcore::{ConsumeRequest, ConsumeResponse, Header, Type};

pub struct Consumer {
    socket: TcpStream,
    offset: u64,
    partition: u32,
    topic: String,
}

impl Consumer {
    pub async fn new(
        server: &str,
        topic: &str,
        offset: u64,
        partition: u32,
    ) -> WombatResult<Consumer> {
        Ok(Consumer {
            socket: TcpStream::connect(server).await?,
            offset,
            partition,
            topic: topic.to_string(),
        })
    }

    pub async fn poll(&mut self) -> WombatResult<ConsumeResponse> {
        Header::new(Type::Consume)
            .write_to(&mut self.socket)
            .await?;
        // TODO(AD) Request multiple partitions and topics on one message.
        let req = ConsumeRequest::new(&self.topic, self.offset, self.partition);
        req.write_to(&mut self.socket).await?;

        let header = Header::read_from(&mut self.socket).await.unwrap();
        match header.kind() {
            Type::Consume => {
                let resp = ConsumeResponse::read_from(&mut self.socket).await.unwrap();
                self.offset = resp.next_offset();
                Ok(resp)
            }
            _ => panic!("unrecognized message type"), // TODO handle
        }
    }
}
