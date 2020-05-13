# Consumer Coordination

## On Startup
* Generate a UUID as the consumer ID

* Add to the consumer registry at:
```
  /group/[consumer group]/[UUID]
```
If this already exists, just try again by generating a new UUID.

Also create the registry ```/group``` and group ```/group/[consumer group]```
if it does not already exist, which are permanent and never deleted.
The consumer UUID is written as an ephemeral znode so when the consumer is lost
the path will be removed.

* Trigger a rebalance of the partitions (below).

* Watch the consumer registry and topic registry for changes to the group
membership and trigger a rebalance when detected


## On Subscribe
Calling ```consumer.Subscibe(topic)``` should add the topic for all members
of the consumer group to subscribe to the topic:
* Add the topic (if it does not already exist) to:
```
  /topic/[group]/[topic]
```
* This will trigger a rebalance (see above)


## Offsets
After a rebalance the consumer starts to pull data from its assigned
partitions:
* Lookup the offset for this partition at
```
  /offset/[group]/[topic]/[partition] -> uint64
```
If this doesnt exist (due to being new) default to zero.

* After pulling data, once the client has processed the data they call
```consumer.Commit(record)```. This then updates the offset registry with the
new commited offset so that after a rebalance another consumer won't pull the
commited data.


## Rebalance
TODO(AD)
Partition registry:
```
  /partitions/[group]/[topic]/[partition] -> [consumer UUID]
```
