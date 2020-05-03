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

        let numbers: Vec<u8> = (0..rng.gen_range(0, 0xff))
            .map(|_| rng.gen_range(0, 0xff))
            .collect();

        println!("produce {}", numbers.len());

        Header::new(Type::Produce)
            .write_to(&mut stream)
            .await
            .unwrap();
        let prod_req = ProduceRequest::new(numbers.clone());
        prod_req.write_to(&mut stream).await.unwrap();

        Header::new(Type::Consume)
            .write_to(&mut stream)
            .await
            .unwrap();
        let cons_req = ConsumeRequest::new(offset);
        cons_req.write_to(&mut stream).await.unwrap();

        let header = Header::read_from(&mut stream).await.unwrap();
        match header.kind() {
            Type::Consume => {
                let resp = ConsumeResponse::read_from(&mut stream).await.unwrap();
                assert_eq!(offset, resp.offset());
                // TODO no clone
                assert_eq!(numbers, resp.payload().clone());
                offset = resp.next_offset();
            }
            _ => (),
        }
    }
}
