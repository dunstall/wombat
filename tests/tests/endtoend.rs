use crc::{crc32, Hasher32};
use rand::Rng;
use std::collections::HashSet;

use wombatclient::{Consumer, Producer};
use wombatcore::ProduceRecord;

#[tokio::test]
#[ignore]
async fn single_partition() {
    let mut rng = rand::thread_rng();
    let mut producer = Producer::new("172.17.0.2:3110").unwrap();

    let mut sent = HashSet::new();

    for _ in 0..100 {
        let val: Vec<u8> = (0..rng.gen_range(0, 0xff))
            .map(|_| rng.gen_range(0, 0xff))
            .collect();

        let mut digest = crc32::Digest::new_with_initial(crc32::IEEE, 0);
        digest.write(val.as_slice());
        sent.insert(digest.sum32());

        let mut record = ProduceRecord::new("mytopic", vec![], val);
        // Manually set to partition 0.
        record.set_partition(1);
        producer.send(record).await.unwrap();
    }

    // TODO(AD) Producer is async. Must have mechanism to wait for all sent.
    // Use close() method (which can also be called in drop to join all threads.
    tokio::time::delay_for(std::time::Duration::from_millis(3000)).await;

    let mut consumer = Consumer::new("172.17.0.2:3110", "mytopic", 0, 1)
        .await
        .unwrap();
    for _ in 0..100 {
        let record = consumer.poll().await.unwrap();
        let mut digest = crc32::Digest::new_with_initial(crc32::IEEE, 0);
        digest.write(record.val().as_slice());
        sent.remove(&digest.sum32());
    }

    // Verify all sent records were received.
    assert_eq!(0, sent.len());
}