use crate::partitionrouter::PartitionRouter;
use crate::result::WombatResult;
use wombatcore::ProduceRecord;

pub struct Producer {
    router: PartitionRouter,
}

impl Producer {
    // TODO(AD) Responsible for handling configuration.
    pub async fn new(server: &str) -> WombatResult<Producer> {
        Ok(Producer {
            router: PartitionRouter::new(),
        })
    }

    pub async fn send(&mut self, record: ProduceRecord) -> WombatResult<()> {
        self.router.route(record);
        Ok(())
    }
}
