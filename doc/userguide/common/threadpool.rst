Thread Pool
===========

SMTK's thread pool pattern (:smtk:`ThreadPool
<smtk::common::ThreadPool>`) constructs a thread pool for
asynchronously executing functors that return a common type. The
number of threads in the pool can be assigned at construction, with
the default number of threads selected by the hardware's concurrency
value. When a functor is passed to the thread pool, a std::future
containing the return type is immediately returned. Multiple instances
of the thread pool can exist simultaneously; each instance will allocate a
user-defined number of threads that will stay idle until the thread
pool is given a task to perform.

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/comon/testing/cxx/UnitTestThreadPool.cxx`.
