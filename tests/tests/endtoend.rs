use rand::Rng;
use tokio::net::TcpStream;

use wombatclient::Producer;
use wombatcore::{ConsumeRequest, ConsumeResponse, Header, ProduceRecord, Type};

#[tokio::test]
async fn end_to_end() {
    let mut producer = Producer::new("localhost:3110").unwrap();
    let mut rng = rand::thread_rng();

    let mut socket = TcpStream::connect("127.0.0.1:3110").await.unwrap();
    let mut offset: u64 = 0;
    loop {
        println!("offset {}", offset);

        let val: Vec<u8> = (0..rng.gen_range(0, 0xff))
            .map(|_| rng.gen_range(0, 0xff))
            .collect();
        let record = ProduceRecord::new("mytopic", vec![], val);
        producer.send(record).await.unwrap();

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
