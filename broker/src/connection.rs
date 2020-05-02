use std::path::Path;
use tokio::net::TcpStream;

use crate::log::{Log, Record};
use wombatcommon::Message;

pub struct Connection {
    socket: TcpStream,
}

impl Connection {
    pub fn new(socket: TcpStream) -> Connection {
        Connection {
            socket: socket,
        }
    }

    pub async fn handle(&mut self) {
        // TODO Must be shared and protected
        let mut log = Log::open(Path::new("segments"), 100000).await.unwrap();

        loop {
            if let Ok(message) = Message::read_from(&mut self.socket).await {
                println!("message recv");
                let record = Record::new(vec![], message.payload().clone());
                log.append(record).await.unwrap();
            } else {
                println!("error on socket");
                return;
            }
        }
    }
}
