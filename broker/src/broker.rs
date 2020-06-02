use crate::result::BrokerResult;

use wombatpartition::PartitionID;

pub struct Broker {}

impl Broker {
    pub fn consume(partition: PartitionID) -> BrokerResult<(Vec<u8>, u64)> {
        Ok((vec![], 0))
    }

    pub fn produce(partition: PartitionID, data: &Vec<u8>) -> BrokerResult<()> {
        Ok(())
    }
}
