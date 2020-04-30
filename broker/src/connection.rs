use std::net::SocketAddr;

use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpStream;

use crate::message::Message;
use crate::message::header::{Header, MESSAGE_HEADER_SIZE};

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
            let header = Header::decode(buf.to_vec()).unwrap();
            let message = Message::new(header);

            println!("Recv {:?} from {}", message, self.addr);

            if let Err(e) = self.socket.write_all(&buf[0..n]).await {
                eprintln!("failed to write to socket; err = {:?}", e);
                return;
            }
        }
    }
}
