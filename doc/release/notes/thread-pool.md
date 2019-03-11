## Introduce a thread pool for multithreaded operation execution

To prevent applications from appearing to "hang" when long-running operations
are executed, a thread pool has been introduced for the managed
launching of operation tasks. The use of a thread pool allows for a
finite number of threads to be continuously reused for subsequent
operations, eliminating the overhead and potential bottleneck of
spawning a new thread for each operation.
