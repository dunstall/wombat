use wombatbroker;

#[tokio::main]
async fn main() {
    // TODO(AD) Make port argument
    wombatbroker::run(3110).await;
}
