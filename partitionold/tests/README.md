# Testing
Tests are split into 3 types:
* Unit - tests a small group of classes, all in memory (uses `InMemorySegment`)
* Ingration - tests larger classes using the filesystem (uses `SystemSegment`)
* Component - tests networked components using a test harness to trigger
all failure modes. So the test harness must be running.
