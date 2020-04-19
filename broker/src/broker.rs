use websocket::sync::Server;
use websocket::OwnedMessage;

pub struct Broker {}

impl Broker {
    pub fn new() -> Broker {
        Broker {}
    }

    pub fn listen(&self) -> std::io::Result<()> {
        let server = Server::bind("0.0.0.0:3110")?;

        for request in server.filter_map(Result::ok) {
            if !request.protocols().contains(&"producer".to_string()) {
                request.reject().unwrap();
                continue;
            }
            let client = request.use_protocol("producer").accept().unwrap();

            let ip = client.peer_addr().unwrap();

            println!("Connection from {}", ip);

            let (mut receiver, mut sender) = client.split()?;

            for message in receiver.incoming_messages() {
                let message = message.unwrap();
                println!("{:?}", message);

                match message {
                    OwnedMessage::Close(_) => {
                        let message = OwnedMessage::Close(None);
                        sender.send_message(&message).unwrap();
                        println!("Client {} disconnected", ip);
                        return Ok(());
                    }
                    OwnedMessage::Ping(ping) => {
                        let message = OwnedMessage::Pong(ping);
                        sender.send_message(&message).unwrap();
                    }
                    _ => sender.send_message(&message).unwrap(),
                }
            }
        }
        Ok(())
    }
}
