use std::collections::HashMap;

use crate::partition::loadbalancer::LoadBalancer;
use crate::partition::route::Route;
use wombatcore::ProduceRecord;

#[derive(Hash, Eq, PartialEq, Debug)]
struct PartitionIndex {
    topic: String,
    partition: u32,
}

// Handles routing a record to the correct partition.
pub struct Router {
    lb: LoadBalancer,
    routes: HashMap<PartitionIndex, Route>,
}

impl Router {
    pub fn new() -> Router {
        Router {
            lb: LoadBalancer::new(),
            routes: HashMap::new(),
        }
    }

    pub async fn route(&mut self, mut record: ProduceRecord) {
        if record.partition() == 0 {
            let partition = if record.key().is_empty() {
                self.lb.next(record.topic())
            } else {
                self.lb.from_key(record.key())
            };
            record.set_partition(partition);
        }

        self.get_route(record.topic(), record.partition())
            .route(record)
            .await;
    }

    fn add_route(&mut self, topic: &str, partition: u32) {
        self.routes.insert(
            PartitionIndex {
                topic: topic.to_string(),
                partition,
            },
            Route::new(topic.to_string(), partition),
        );
    }

    fn get_route(&mut self, topic: &str, partition: u32) -> &mut Route {
        if !self.route_exists(topic, partition) {
            self.add_route(topic, partition);
        }

        let index = PartitionIndex {
            topic: topic.to_string(),
            partition,
        };
        if let Some(route) = self.routes.get_mut(&index) {
            route
        } else {
            // Already added so this can never happen.
            panic!("route should exist");
        }
    }

    fn route_exists(&mut self, topic: &str, partition: u32) -> bool {
        self.routes.contains_key(&PartitionIndex {
            topic: topic.to_string(),
            partition,
        })
    }
}
