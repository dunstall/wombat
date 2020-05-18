mod server;

use std::net::{IpAddr, Ipv4Addr, SocketAddr};

pub async fn run(port: u16) {
    // TODO(AD) IP as argument
    let addr = SocketAddr::new(IpAddr::V4(Ipv4Addr::new(0, 0, 0, 0)), port);
    let server = server::Server::new();
    // TODO(AD) messy
    std::process::exit(match server.listen(addr).await {
        Ok(_) => 0,
        Err(err) => {
            eprintln!("error: {:?}", err);
            1
        }
    });
}