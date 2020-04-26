use std::error::Error;
use std::net::TcpStream;
use std::string::String;

use websocket::client::sync::Client;
use websocket::client::ClientBuilder;
use websocket::Message;

pub trait Connection {
    fn recv(&self) -> Result<&[u8], Box<dyn Error>>;
    fn send(&mut self, data: &[u8]) -> Result<(), Box<dyn Error>>;
}

pub struct WSConnection {
    client: Client<TcpStream>,
}

impl WSConnection {
    pub fn connect(protocol: String) -> Result<WSConnection, Box<dyn Error>> {
        let client = ClientBuilder::new("ws://localhost:3110")?
            .add_protocol(protocol.clone())
            .connect_insecure()?;
        Ok(WSConnection { client: client })
    }
}

impl Connection for WSConnection {
    fn recv(&self) -> Result<&[u8], Box<dyn Error>> {
        Ok(b"test")
    }

    fn send(&mut self, data: &[u8]) -> Result<(), Box<dyn Error>> {
        self.client.send_message(&Message::binary(data))?;
        Ok(())
    }
}
