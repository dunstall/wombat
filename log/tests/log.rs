use wombatlog::{Log, SystemSegment};

use rand::Rng;
use std::time::Instant;
use tempdir::TempDir;

#[tokio::test]
#[ignore]
async fn random_data() {
    let dir = tmp_dir();
    let mut log = Log::<SystemSegment>::open(dir.path(), 1_000_000_000).await.unwrap();

    let mut rng = rand::thread_rng();

    for n in 0..1000 {
        let numbers: Vec<u8> = (0..0xff).map(|_| rng.gen_range(0, 0xff)).collect();

        log.append(&numbers).await.unwrap();
        assert_eq!(numbers, log.lookup(n * 0xff, 0xff).await.unwrap());
    }
}

#[tokio::test]
#[ignore]
async fn benchmark() {
    let dir = tmp_dir();
    let mut log = Log::<SystemSegment>::open(dir.path(), 1_000_000_000).await.unwrap();

    let mut rng = rand::thread_rng();
    let numbers: Vec<u8> = (0..0xff).map(|_| rng.gen_range(0, 0xff)).collect();

    let now = Instant::now();
    for _ in 0..100_000 {
        log.append(&numbers).await.unwrap();
    }
    println!("Append time {}", now.elapsed().as_secs());

    for n in 0..100_000 {
        assert_eq!(numbers, log.lookup(n * 0xff, 0xff).await.unwrap());
    }
    println!("Append and lookup time {}", now.elapsed().as_secs());
}

fn tmp_dir() -> TempDir {
    TempDir::new("dingolog").unwrap()
}
