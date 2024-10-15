Operation Subsystem
===================

Proper GIL locking
------------------

The scheme SMTK uses for running operations from python
has changed to address potential deadlocks.
This should not require any changes to code that depends
on SMTK and may fix issues with your python operations.

PyOperation methods must not have ownership of the GIL since
the ``PYBIND11_OVERLOAD`` macro acquires it (resulting in a deadlock
if the GIL is already held). However, PyOperation instances may
be called from either C++ or from Python. If called from Python,
we must release the GIL held by python so it can be acquired.
If called from C++ code, we must not release the GIL since we
do not already hold it.

Instead of always releasing the GIL inside PyOperation, release
it from within the python bindings. This way, we do not release
the GIL when it is not held (causing assertion failures on macos
and perhaps other platforms).

In addition to releasing and then re-acquiring the GIL when
calling python operation code from python, we must ensure
that any python code is executed on the main application thread
(i.e., the GUI thread) even when invoked from a separate thread.
This way, python operations can be launched in a separate thread
that yields until all the required resources are locked and then
run in the GUI thread with the proper GIL locking.
