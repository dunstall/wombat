use crate::partition::connection::Connection;
use wombatcore::ProduceRecord;

pub struct Partition {
    connection: Connection
}

// TODO this must have the thread - as send just queues and evenautlly sends
// TODO just send each record for now - batch later

impl Partition {
    pub fn new() -> Partition {
        Partition {
            connection: Connection{}
        }
    }

    pub fn send(&mut self, record: ProduceRecord) {}
}
