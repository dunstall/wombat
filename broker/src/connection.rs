use std::net::SocketAddr;

use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpStream;

use crate::message::header::{Header, MESSAGE_HEADER_SIZE};
use crate::message::Message;

pub struct Connection {
    socket: TcpStream,
    addr: SocketAddr,
}

impl Connection {
    pub fn new(socket: TcpStream, addr: SocketAddr) -> Connection {
        Connection {
            socket: socket,
            addr: addr,
        }
    }

    pub async fn handle(&mut self) {
        // TODO have one buffer allocated once (for payload too)?

        let mut buf = [0; MESSAGE_HEADER_SIZE];
        loop {
            let n = match self.socket.read_exact(&mut buf).await {
                // socket closed
                Ok(n) if n == 0 => return,
                Ok(n) => n,
                Err(e) => {
                    // TODO result?
                    eprintln!("failed to read from socket; err = {:?}", e);
                    return;
                }
            };

            // TODO for now just panic
            let header = Header::decode(buf).unwrap();

            let mut payload = Vec::new();
            payload.resize(header.payload_size() as usize, 0);
            self.socket
                .read_exact(payload.as_mut_slice())
                .await
                .unwrap();

            let message = Message::new(header, payload);

            println!("Recv {:?} from {}", message, self.addr);

            if let Err(e) = self.socket.write_all(&buf[0..n]).await {
                eprintln!("failed to write to socket; err = {:?}", e);
                return;
            }
        }
    }
}
