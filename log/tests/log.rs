use wombatlog::{Log, SegmentManager, SystemSegmentManager};

use rand::Rng;
use std::time::Instant;
use tempdir::TempDir;

#[test]
#[ignore]
fn random_data() {
    let dir = tmp_dir();

    let mut manager: std::boxed::Box<(dyn SegmentManager + 'static)> =
        Box::new(SystemSegmentManager::new(&dir.path()).unwrap());

    let mut log = Log::open(&mut manager, 1_000_000_000).unwrap();

    let mut rng = rand::thread_rng();

    for n in (0..100_000) {
        let numbers: Vec<u8> = (0..0xff).map(|_| rng.gen_range(0, 0xff)).collect();

        log.append(&numbers).unwrap();
        assert_eq!(numbers, log.lookup(0xff, n * 0xff).unwrap());
    }
}

#[test]
#[ignore]
fn benchmark() {
    let dir = tmp_dir();

    let mut manager: std::boxed::Box<(dyn SegmentManager + 'static)> =
        Box::new(SystemSegmentManager::new(&dir.path()).unwrap());

    let mut log = Log::open(&mut manager, 1_000_000_000).unwrap();

    let mut rng = rand::thread_rng();
    let numbers: Vec<u8> = (0..0xff).map(|_| rng.gen_range(0, 0xff)).collect();

    let now = Instant::now();
    for _ in (0..10_000_000) {
        log.append(&numbers).unwrap();
    }
    println!("Append time {}", now.elapsed().as_secs());

    for n in (0..10_000_000) {
        assert_eq!(numbers, log.lookup(0xff, n * 0xff).unwrap());
    }
    println!("Append and lookup time {}", now.elapsed().as_secs());
}

fn tmp_dir() -> TempDir {
    TempDir::new("dingolog").unwrap()
}
