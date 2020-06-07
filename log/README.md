# Log Module
Handles representing multiple segment files as a single log.

## Performance
Using Tokio seems to slow down appending to a file significantly. Once the
rest of the broker is working this is likely a bottleneck to optimize. Such as
remove Tokio as instead have a thread pool to append to files.
