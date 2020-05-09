use rand::Rng;

use wombatclient::Producer;
use wombatcore::ProduceRecord;

#[tokio::main]
async fn main() {
    let mut rng = rand::thread_rng();

    let mut producer = Producer::new("localhost:3110").unwrap();
    loop {
        let val: Vec<u8> = (0..rng.gen_range(0, 0xff))
            .map(|_| rng.gen_range(0, 0xff))
            .collect();
        // Use an empty key to load balance records across partitions.
        let record = ProduceRecord::new("mytopic", vec![], val);
        producer.send(record).await.unwrap();
    }
}
