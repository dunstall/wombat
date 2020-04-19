use std::io::prelude::*;
use std::net::{SocketAddr, TcpStream};

pub struct Consumer {
    addr: SocketAddr,
}

impl Consumer {
    pub fn new(addr: SocketAddr) -> Consumer {
        Consumer { addr: addr }
    }

    pub fn poll(&self) -> std::io::Result<()> {
        let mut stream = TcpStream::connect(self.addr)?;

        stream.write(&[1])?;
        Ok(())
    }
}
