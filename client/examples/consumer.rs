use wombatclient::Consumer;

#[tokio::main]
async fn main() {
    let mut consumer = Consumer::new("localhost:3110", "mytopic", 0, 0)
        .await
        .unwrap();
    loop {
        let record = consumer.poll().await.unwrap();
        println!("{}", record.next_offset());
    }
}
