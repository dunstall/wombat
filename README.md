# Wombat
Distributed messaging system in Rust

## In progress

## Design Ideas

### Cassandra
* Load balance consumers
* Membership and failure detection
* Replica syncronization: Route put requests to other replicas
* Overload handling: Alleviate heavily loaded nodes by moving lightly loaded
nodes around the ring (see Chord) (5.1)

* Configure the consistency guarantees per request (ie quorum). So can specify
asyncrous (put/get from 1 node - eventually consistent) or syncrous (with quorum
for stronger guarantees)

* Consistent hashing: organize nodes into a ring with one node assigned coordinator
for a key (by walking around the ring). Replicate this on the next N nodes round
the ring (preference list in Dynamo)
  * This allows adding/removal of nodes to only affect neighbours
  * For membership detection probably just use ZooKeeper and can use something
more advanced. Cassandra uses ZooKeeper for electing the leader who assigns ranges. (5.2)
Also persists all metadata in ZooKeeper. For starting can watch ZooKeeper for
membership changes. Nodes can then cache membership info for efficient request
routing. Also nodes should persist their own position in the ring locally (eg
if the process crashes)
  * Replication factor per instance

* See 5.4 for bootstaping - points out that should not immediately rebalance on
a node failure as often transient. Maybe just use a long ZooKeeper session timeout
so ephemeral nodes are only removed if the node has been unreachable for a long period.

* Collision detection is a bigger challenge for the log as is append only - possibly
if a collision is detected just append both versions so there is no data loss
(even if this can result in duplicates with a record at multiple offsets). Or just
only write to the leader (since it should be heavily partitioned this shouldnt be
a bottleneck - can use ZK to elect leader)

* Bootstrap: New nodes should be assigned to aleviate heavily loaded nodes (potentially
use vnodes from Dynamo?) Or just dont use consistent hashing and assign partitions
evenly in Kafka style (though avoid range partitioning - if a node joins just take
partitions from most loaded nodes)

* Impl:
  * Partitioning module
  * Cluster membership and failure detection module
  * Storage engine module

  * SEDA archetacture (!!)

Routing request module state machine:
1 Identify node(s) owning key
2 Route request to these nodes and wait for response (only wait if sync?)
3 Fail request if timeout
4 Return latest response to client - eg if receive resp from 3 replicas take the
latest
5 Shedule repare of replicas with stale data (which may be done in the case with
just send the whole segment)
Allows async (fire and forget) vs sync (wait for quorum): configure on per request basis

### Startup
* Use systemctl for startup (and restarting) for VMs - and maybe also provide a
Docker version.


## Language
This was started in Rust - though currently porting to C++ due to Rust being to
restrictive (especially around not being able to use UNIX C functions like epoll and sendfile).

## Partition
The log is partitioned with each node assigned is assigned N partitions.

TODO decide on how to coordinate (P2P or a master).
TODO use ZK for persistent state

## Replication
Each partition is owned by a node are replicated on a further M nodes.

### On Rebalance
When a node becomes a replica for a partition it must request the log for this
partition. Each log is split into multiple segments (128MB) so use sendfile to
transfer segments.

### On Write
Each write is configured with a consistency requirement for the number of nodes
to write to (so the client can use async or a quorum). All writes are routed
to the owner of the partition (as conflicts would be difficult as the log is
append only).

* 1 Write the data to the local log on the partition
* 2 Identify the replica nodes
* 3 Route the request to these nodes and wait for N responses (where N is the
number of configured nodes)
* 4 On timeout return an error to the client (maybe start with only allowing
async)

### On Read
Reads should always be routed to replicas (the client should cache this info
and know to route reads to replicas). If the 


 This owner first appends to its local log - then

The log on each node is split into multiple segments (128MB?) so should be able
to use sendfile to just transfer whole segments concurrently when a new node
starts up.

For each request just route to replicas.

To avoid collisions just elect a master for each partition with ZooKeeper which
is the only writable node (route requests to reader nodes). This should not
be a bottleneck since can partition heavily).

To select replicas just choose the least loaded nodes (which is probably easier
that consistent hashing - though may require a master node to make these
assignment decisions)

## Concurrency
Each partition should only be consumed by a small number of clients (as using
the Kafka consumer group model). So each partition can just have a single thread
or process to handle the reads and writes - so blocking file access may not
be an issue.

Having a process per partition may work (vnodes) - with multiple partitions
per node (which makes replication harder as cannot replicate to the same
physical node - but this is managable with some master to handle this logic)

* Linux can handle atomic writes to files under 512(?) bytes

TODO as part of log module have a method to send log to another node

## TODO
* For testing look into ducktape (see https://github.com/apache/kafka/tree/trunk/tests)
