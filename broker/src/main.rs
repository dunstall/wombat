mod broker;

fn main() -> std::io::Result<()> {
    println!("Running Wombat broker");

    let mut broker = broker::Broker::new();
    broker.listen()?;

    Ok(())
}
