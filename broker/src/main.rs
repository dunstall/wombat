use wombatbroker;

#[tokio::main]
async fn main() {
    wombatbroker::run().await;
}
