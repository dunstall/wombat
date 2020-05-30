# Log Module
* Segment files of 1GB (configurable)
* Basic interface of
  * Log(retention)
  * Lookup(offset, size) -> vec
  * Write(vec)
* Flush to disk after certain amount of data or time passed (configurable)
* In memory index of offset -> segment file

* Need component testing using real FS with benchmarks
