use rand::Rng;
use tokio::net::TcpStream;

use wombatcore::{ConsumeRequest, ConsumeResponse, Header, ProduceRequest, Type};

#[tokio::test]
async fn end_to_end() {
    let mut socket = TcpStream::connect("127.0.0.1:3110").await.unwrap();
    let mut offset: u64 = 0;
    loop {
        println!("offset {}", offset);

        send_produce_request(&mut socket).await;
        send_consume_request(offset, &mut socket).await;

        let header = Header::read_from(&mut socket).await.unwrap();
        match header.kind() {
            Type::Consume => {
                let resp = ConsumeResponse::read_from(&mut socket).await.unwrap();
                // TODO(AD) Verify the data received - use a set of CRC32s of the messages sent
                offset = resp.next_offset();
                println!("{}", offset);
            }
            _ => panic!("unrecognized message type"),
        }
    }
}

async fn send_consume_request(offset: u64, socket: &mut TcpStream) {
    Header::new(Type::Consume).write_to(socket).await.unwrap();
    // TODO(AD) Request multiple partitions on one message.
    let cons_req = ConsumeRequest::new("mytopic", offset, 0);
    cons_req.write_to(socket).await.unwrap();
}

async fn send_produce_request(socket: &mut TcpStream) {
    let (key, val) = generate_message(0xff);

    println!("produce {}", key.len());

    Header::new(Type::Produce).write_to(socket).await.unwrap();
    // TODO(AD) For now use empty key to get partition 0.
    let prod_req = ProduceRequest::new("mytopic", vec![], key);
    prod_req.write_to(socket).await.unwrap();
}

fn generate_message(max_len: u64) -> (Vec<u8>, Vec<u8>) {
    let mut rng = rand::thread_rng();
    let key: Vec<u8> = (0..rng.gen_range(0, 0xff))
        .map(|_| rng.gen_range(0, 0xff))
        .collect();
    let val: Vec<u8> = (0..rng.gen_range(0, max_len))
        .map(|_| rng.gen_range(0, 0xff))
        .collect();

    (key, val)
}
