use std::collections::HashMap;

use crate::loadbalancer::LoadBalancer;
use crate::route::Route;
use wombatcore::ProduceRecord;

// Handles routing a record to the correct partition.
pub struct PartitionRouter {
    lb: LoadBalancer,
    routes: HashMap<String, Route>,
}

impl PartitionRouter {
    pub fn new() -> PartitionRouter {
        PartitionRouter {
            lb: LoadBalancer::new(),
            routes: HashMap::new(),
        }
    }

    pub fn route(&mut self, mut record: ProduceRecord) {
        if record.partition() == 0 {
            let partition = if record.key().is_empty() {
                self.lb.next(record.topic())
            } else {
                self.lb.from_key(record.key())
            };
            record.set_partition(partition);
        }

        // TODO(AD) send to correct route - add if not exists
        if let Some(route) = self.routes.get_mut("test") {
            route.route(record);
        } else {
            // TODO(AD) add route
        }
    }

    fn add(&mut self, topic: &str, partition: u32) {
        let queue = Route::new(topic.to_string(), partition);
        // TODO(AD) Object with topic and partition as key to ensure unique.
        self.routes.insert(topic.to_string() + &partition.to_string(), queue);
    }
}
