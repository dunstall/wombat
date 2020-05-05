use tokio::net::TcpStream;

use crate::partitioner::Partitioner;
use crate::result::WombatResult;
use wombatcore::{Header, ProduceRecord, Type};

pub struct Producer {
    partitioner: Partitioner,
    socket: TcpStream,
}

impl Producer {
    pub async fn new(server: &str) -> WombatResult<Producer> {
        Ok(Producer {
            partitioner: Partitioner::new(),
            socket: TcpStream::connect(server).await?,
        })
    }

    pub async fn send(&mut self, mut record: ProduceRecord) -> WombatResult<()> {
        if record.partition() == 0 {
            let partition = if record.key().is_empty() {
                self.partitioner.next(record.topic())
            } else {
                self.partitioner.from_key(record.key())
            };
            record.set_partition(partition);
        }
        Header::new(Type::Produce)
            .write_to(&mut self.socket)
            .await?;
        record.write_to(&mut self.socket).await?;

        Ok(())
    }
}
