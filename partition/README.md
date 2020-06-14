# Log Module

## Design
Wombat should be heavily partitioned - each partition (where each node
may have many partitions) owns a log that is accessed by a single thread.

The log has no knowledge of the structure of the data but simply provides an
interface with operations:
* `Append(data)`
* `Lookup(offset, size) -> data`

### Segments
Each log consists of segments of maximum size 128MB. The purpose of this is:
* Allow removal of expired segments
* Easily replicate the log by sending sements concurrently using kernel level
copying (`sendfile`). This is to be used when a node becomes a replica for the
partition or a segment is corrupted and requests a new clone
* If a record is detected as corrupted (when a CRC check fails) the log can
request this segment from a replica rather than try to recover data from that
segment

### Retention Policy
Logs have a retention policy to remove expired segments after a given time
(only removing full segments - never the one currently writing to). Need to
decide whether to:
* Have an additional method `Expire(since)` so the decision about when to call
can be moved out of this module making testing easier
* Add a background thread to cleanup when needed (though this requires making
the log thread safe which is not an appealing option)
* Check for expired segments on `Append`/`Lookup`, though this may slow down
these operations unexpectedly especially if many segements expire

### Generics
To make testing easier (both for this module and higher layers) `Segment` is
an abstract base class. Since the log must handle constructing these segments
rather passing as a parameter the segment can be a template (`Log<S>`) so for
testing can use an in-memory segment rather than one that uses the file system.

### File Transfer
Transfering the log for replication requires sending segments concurrently with
`sendfile` so this is a log level operation (as segments are encapsulated). For
sending segments the client (another broker) should be able to specify the
segments then want (eg if segment 5 is corrupted so need it from a replica).

### Corruption
Each record contains a CRC to detect corruption which is checked on lookup. If
this occurs the log requests the segment from a replica - for forwards lookups
to a replica until it is repaired.

### Append
Append is only called on the partition coordinator (never replicas - maybe add
option to append to replicas and just forward to master). This is to avoid
conflicts that would be hard to resolve in an append only log.

First push the data to a queue to be handled by a background thread - then if
that succeeds write to the local log.

In the background the request should be written to all replicas (received from
the cluster membership module)

#### Receive Replica Request
The partition listens for incoming requests from the coordinator:
* 1 - Check the local log is upto the same offset as the new request
* 2 - If not buffer incoming requests while requesting the missing segments from
the coordinator or another replica
* 3 - If the buffer exceeds the segment size (128MB) discard and just request
the segment
* 4 - Once upto data apply the buffer to the log
