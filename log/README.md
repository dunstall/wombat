# Log Module
Handles representing multiple segment files as a single log.

## Performance
Using Tokio seems to slow down appending to a file significantly. Once the
rest of the broker is working this is likely a bottleneck to optimize. Such as
remove Tokio as instead have a thread pool to append to files.

Results:
* Time to append 0xff x 100,000 bytes: 9 seconds
* Time to lookup 0xff x 100,000 bytes (in order): 8 seconds
This is clearly odd if lookup is faster than append.
