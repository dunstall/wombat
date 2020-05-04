# System Integration & Performance Testing

## Tests
* End to end: a single client listening on all partitions on a single topic and verify all data sent is received. Messages upto 2 x 0xff bytes
* Big messages: same as above with messages upto 0x1fffff (2 MB).
* Multi Client: multiple client threads on a single machine, with 1 partition each (TODO This will be replaced once client coordination added)

Future tests
* Multiple clients distributed over many machines/containers with multiple topics and consumer groups
* Multiple servers distributed over many machines/containers
