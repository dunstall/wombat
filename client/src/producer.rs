use crate::partition::Router;
use crate::result::WombatResult;
use wombatcore::ProduceRecord;

pub struct Producer {
    router: Router,
}

impl Producer {
    // TODO(AD) Responsible for handling configuration.
    pub fn new(server: &str) -> WombatResult<Producer> {
        Ok(Producer {
            router: Router::new(),
        })
    }

    pub async fn send(&mut self, record: ProduceRecord) -> WombatResult<()> {
        self.router.route(record).await;
        Ok(())
    }
}
