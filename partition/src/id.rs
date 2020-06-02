pub struct PartitionID {
    topic: String,
    partition: u64,
}

impl PartitionID {
    pub fn new(topic: String, partition: u64) -> PartitionID {
        PartitionID{
            topic, partition
        }
    }

    pub fn topic(&self) -> &String {
        &self.topic
    }

    pub fn partition(&self) -> u64 {
        self.partition
    }
}
