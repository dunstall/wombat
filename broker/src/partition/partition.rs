pub struct Partition;

impl Partition {
    pub async fn open(topic: &str, n: u32) -> Partition {
        Partition {}
    }

    pub fn map_to_partition(key: &Vec<u8>) -> u32 {
        0
    }

    pub async fn read(&mut self, offset: u64) -> (u64, Vec<u8>, Vec<u8>) {
        (0, vec![], vec![])
    }

    pub async fn write(&mut self, key: &Vec<u8>, val: &Vec<u8>) -> u64 {
        0 // return offset
    }
}
