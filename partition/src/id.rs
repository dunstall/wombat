pub struct PartitionID {
    topic: String,
    partition: u64,
}

impl PartitionID {
    pub fn new(topic: String, partition: u64) -> PartitionID {
        PartitionID { topic, partition }
    }

    pub fn topic(&self) -> &String {
        &self.topic
    }

    pub fn partition(&self) -> u64 {
        self.partition
    }

    pub fn to_string(&self) -> String {
        self.topic.clone() + &self.partition.to_string()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn getters() {
        let id = PartitionID::new("mytopic".to_string(), 0xf57d);
        assert_eq!("mytopic", id.topic());
        assert_eq!(0xf57d, id.partition());
    }

    #[test]
    fn to_string() {
        let id = PartitionID::new("mytopic".to_string(), 1234);
        assert_eq!("mytopic1234", id.to_string());
    }
}
