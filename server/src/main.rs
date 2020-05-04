use wombatbroker;

#[tokio::main]
async fn main() {
    wombatbroker::run(3110).await;
}
