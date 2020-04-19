use std::net::TcpListener;
use std::net::TcpStream;

pub struct Broker {}

impl Broker {
    pub fn new() -> Broker {
        Broker {}
    }

    pub fn listen(&self) -> std::io::Result<()> {
        let listener = TcpListener::bind("0.0.0.0:3110")?;

        // Accept connections and process them serially.
        for stream in listener.incoming() {
            match stream {
                Ok(stream) => {
                    // TODO new thread?
                    self.handle_client(stream);
                }
                Err(e) => {
                    println!("accept error {}", e)
                }
            }
        }
        Ok(())
    }

    fn handle_client(&self, stream: TcpStream) {
        println!("new client");
    }
}
