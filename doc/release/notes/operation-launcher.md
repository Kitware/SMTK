## Add infrastructure for a centralized operation launcher

Operation managers now support instances of
`smtk::operation::Launcher`, functors designed to launch
operations. Multiple launch types are supported, and the default
launcher executes an operation on a concurrent thread.

There is also a Qt-enabled launcher that executes on a child thread
and signals on the main thread when operations are launched. The
signaling launcher will facilitate progress management in the future.
