use wombatbroker;

#[tokio::main]
async fn main() {
    println!("Running Wombat broker");

    wombatbroker::run().await;
}
