use std::error::Error;
use std::net::TcpStream;

use websocket::client::sync::Client;
use websocket::client::ClientBuilder;
use websocket::Message;

pub struct Producer {
    client: Client<TcpStream>,
}

impl Producer {
    pub fn new(url: &str) -> Result<Producer, Box<dyn Error>> {
        let client = ClientBuilder::new(url)?
            .add_protocol("producer")
            .connect_insecure()?;
        Ok(Producer { client: client })
    }

    pub fn send(&mut self, data: &[u8]) -> Result<(), Box<dyn Error>> {
        self.client.send_message(&Message::binary(data))?;
        Ok(())
    }
}

impl Drop for Producer {
    fn drop(&mut self) {
        self.client.shutdown().unwrap();
    }
}
