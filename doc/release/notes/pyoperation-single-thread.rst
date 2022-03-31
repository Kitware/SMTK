Make PyOperations execute on the main thread by default
-------------------------------------------------------

Python operations, including imported python operations, run on the
main gui thread by default, to avoid possible deadlocks because of
the Python global interpreter lock.
