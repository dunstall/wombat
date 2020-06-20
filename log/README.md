# Log Module

## Design
The log has no knowledge of the structure of the data but simply provides an
interface with operations:
* `Append(data)`
* `Lookup(offset, size) -> data`

### Segments
Each log consists of segments of maximum size 128MB. This supports:
* Remove expired segments (retention policy),
* If the partitioner detects a record is currupted (by checking its CRC) the
segment can be requested from another replica
*TODO* This may not be needed and just request offset and record size until
fixed.

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
