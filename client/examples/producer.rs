use wombatclient::Producer;
use wombatcore::ProduceRecord;

#[tokio::main]
async fn main() {
    let mut producer = Producer::new("localhost:3110").await.unwrap();
    loop {
        let record = ProduceRecord::new("mytopic", vec![], b"test-val".to_vec());
        producer.send(record).await.unwrap();
    }
}
