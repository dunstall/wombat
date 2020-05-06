use std::collections::VecDeque;
use std::sync::mpsc::{channel, Receiver, Sender};
use std::thread;

use crate::partition::Partition;
use wombatcore::ProduceRecord;

pub struct Route {
    topic: String,
    partition: u32,
    queue: VecDeque<ProduceRecord>,
    send: Sender<ProduceRecord>,
    // recv: Receiver<ProduceRecord>,
}

impl Route {
    pub fn new(topic: String, partition: u32) -> Route {
        let (send, recv) = channel::<ProduceRecord>();

        let mut q = Route {
            topic,
            partition,
            queue: VecDeque::new(),
            send,
        };
        q.run(recv);
        q
    }

    pub fn route(&mut self, record: ProduceRecord) {
        self.send.send(record).unwrap();  // Never blocks.
    }

    fn run(&mut self, recv: Receiver<ProduceRecord>) {
        // This will stop when send is dropped.
        // TODO(AD) Join?
        let mut partition = Partition::new();
        thread::spawn(move || loop {
            let record = recv.recv().unwrap();
            partition.send(record);
        });
    }
}
