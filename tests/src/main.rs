use std::time;
use tokio::net::TcpStream;
use tokio::time::delay_for;

use wombatcommon::{ConsumeRequest, ConsumeResponse, Header, ProduceRequest, Type};

#[tokio::main]
async fn main() {
    let mut stream = TcpStream::connect("127.0.0.1:3110").await.unwrap();
    loop {
        Header::new(Type::Produce)
            .write_to(&mut stream)
            .await
            .unwrap();

        let prod_req = ProduceRequest::new(vec![0, 1, 2, 3]);
        prod_req.write_to(&mut stream).await.unwrap();

        Header::new(Type::Consume)
            .write_to(&mut stream)
            .await
            .unwrap();

        let cons_req = ConsumeRequest::new(0);
        cons_req.write_to(&mut stream).await.unwrap();

        let header = Header::read_from(&mut stream).await.unwrap();
        match header.kind() {
            Type::Consume => {
                let resp = ConsumeResponse::read_from(&mut stream).await.unwrap();
            }
            _ => (),
        }

        delay_for(time::Duration::from_millis(50)).await;
    }
}
