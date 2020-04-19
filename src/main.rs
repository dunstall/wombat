mod broker;

fn main() -> std::io::Result<()> {
    println!("Running Wombat broker");

    let broker = broker::Broker::new();
    broker.listen()?;

    Ok(())
}
