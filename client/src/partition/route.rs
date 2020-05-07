use tokio::sync::mpsc::{channel, Receiver, Sender};

use crate::partition::connection::Connection;
use wombatcore::ProduceRecord;

pub struct Route {
    topic: String,
    partition: u32,
    send: Sender<ProduceRecord>,
}

impl Route {
    pub fn new(topic: String, partition: u32) -> Route {
        let (send, recv) = channel::<ProduceRecord>(2000);

        let mut q = Route {
            topic,
            partition,
            send,
        };
        q.run(recv);
        q
    }

    pub async fn route(&mut self, record: ProduceRecord) {
        self.send.send(record).await.unwrap(); // Never blocks.
    }

    fn run(&mut self, mut recv: Receiver<ProduceRecord>) {
        // This will stop when send is dropped.
        // TODO(AD) Join?
        // TODO(AD) Use 'real' thread
        tokio::spawn(async move {
            let mut partition = Connection::new().await;
            loop {
                let record = recv.recv().await.unwrap();
                partition.send(record).await;
            }
        });
    }
}
