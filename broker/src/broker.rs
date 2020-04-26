use std::net::TcpStream;
use std::option::Option;
use std::vec::Vec;

use websocket::server::upgrade::sync::Buffer;
use websocket::server::upgrade::WsUpgrade;
use websocket::sync::Server;
use websocket::OwnedMessage;

pub struct Broker {
    logs: Vec<Vec<u8>>,
}

impl Broker {
    pub fn new() -> Broker {
        Broker { logs: Vec::new() }
    }

    pub fn listen(&mut self) -> std::io::Result<()> {
        let server = Server::bind("0.0.0.0:3110")?;

        for req in server.filter_map(Result::ok) {
            self.handle_req(req)?;
        }
        Ok(())
    }

    fn handle_req(&mut self, req: WsUpgrade<TcpStream, Option<Buffer>>) -> std::io::Result<()> {
        if !req.protocols().contains(&"producer".to_string()) {
            req.reject().unwrap();
            return Ok(());
        }
        let client = req.use_protocol("producer").accept().unwrap();

        let ip = client.peer_addr().unwrap();

        println!("Connection from {}", ip);

        let (mut receiver, mut sender) = client.split()?;

        for msg in receiver.incoming_messages() {
            let msg = msg.unwrap();
            println!("{:?}", msg);

            self.handle_msg(msg);
        }

        Ok(())
    }

    fn handle_msg(&mut self, msg: OwnedMessage) {
        match msg {
            OwnedMessage::Close(_) => {
                // let msg = OwnedMessage::Close(None);
                // sender.send_msg(&msg).unwrap();
                // println!("Client {} disconnected", ip);
            }
            OwnedMessage::Ping(ping) => {
                // let msg = OwnedMessage::Pong(ping);
                // sender.send_msg(&msg).unwrap();
            }
            OwnedMessage::Binary(data) => {
                println!("binary data {:?}", data);

                self.logs.push(data)
            }
            OwnedMessage::Text(data) => {
                println!("text data {:?}", data);
            }
            OwnedMessage::Pong(pong) => {
                println!("pong data {:?}", pong);
            }
        }
    }
}
