# Wombat
Distributed messaging system

## Design
Wombat combines ideas from a number of distributed systems, including:
* SEDA archetecture from the Cassandra paper. This splits the system into multiple stages (each with their own thread) that communicate via event queues. The typical flow of data is
  * The lister stage accepts requests (over TCP)
  * The request is routed to the appropriate partition. This partition owns its own log to avoid worrying about syncronization of the file.
  * The response is routed to a responder stage that writes the response to the client.
* Client coordination using ZooKeeper

The broker is written in C++ and the client in Golang, where each is split into multiple modules.

## In Progresss

## Build
Run:
```
./bin/bazelisk build ...
```

And to run unit and integration tests run:
```
./bin/bazelisk test ...
```

Note bazelisk is compiled for Linux x64. If needed install the correct binary
from [Bazelisk](https://github.com/bazelbuild/bazelisk).
