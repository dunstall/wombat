use rand::{thread_rng, Rng};
use std::time;
use tokio::net::TcpStream;
use tokio::time::delay_for;

use wombatcommon::{ConsumeRequest, ConsumeResponse, Header, ProduceRequest, Type};

#[tokio::main]
async fn main() {
    let mut stream = TcpStream::connect("127.0.0.1:3110").await.unwrap();
    let mut rng = rand::thread_rng();
    let mut offset: u64 = 0;
    loop {
        println!("offset {}", offset);

        let key: Vec<u8> = (0..rng.gen_range(0, 0xff))
            .map(|_| rng.gen_range(0, 0xff))
            .collect();
        let val: Vec<u8> = (0..rng.gen_range(0, 0xff))
            .map(|_| rng.gen_range(0, 0xff))
            .collect();

        println!("produce {}", key.len());

        Header::new(Type::Produce)
            .write_to(&mut stream)
            .await
            .unwrap();
        let prod_req = ProduceRequest::new("mytopic", val.clone(), key.clone());
        prod_req.write_to(&mut stream).await.unwrap();

        Header::new(Type::Consume)
            .write_to(&mut stream)
            .await
            .unwrap();
        let cons_req = ConsumeRequest::new("mytopic", offset, 0);
        cons_req.write_to(&mut stream).await.unwrap();

        let header = Header::read_from(&mut stream).await.unwrap();
        match header.kind() {
            Type::Consume => {
                let resp = ConsumeResponse::read_from(&mut stream).await.unwrap();
                // TODO(AD) no clone
                // assert_eq!(offset, resp.offset());
                // assert_eq!(key, resp.key().clone());
                // assert_eq!(key, resp.val().clone());
                offset = resp.next_offset();
                println!("{}", offset);

            }
            _ => println!("bad type"),
        }
    }
}
