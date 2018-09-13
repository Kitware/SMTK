Anatomy of ParaView
-------------------

In order to use SMTK within ParaView, it is important to understand
some of ParaView's design.
A core premise of ParaView is that the user interface will frequently
be a separate process from rendering and I/O tasks, as these must be
run in a distributed-memory parallel environment.
Thus, ParaView has a client process and 1 or more server processes.
Even when running in serial (i.e., non-parallel) mode, ParaView makes a
distinction between tasks run on the client and the server(s).

ParaView's design has the client create and manage objects on the server.
Nearly all objects on the server are subclasses of vtkObject and thus can
be managed using wrappings created by VTK (similar to Python wrappers).
These wrappers are called client-server wrappers and allow ParaView to
serialize, deserialize, and remotely invoke methods on server-side objects.

Nearly all objects on the server have a matching instance on each server process.
For example, if the client creates a file-reader and an actor to display data from the file,
it will instruct each server to create a reader and an actor; and have each server attach the
reader and actor — even though some of the servers' readers may not have data for that
actor to display because the data is unevenly distributed.

This pattern simplifies the work the client must do,
however it also means that server-side objects should rarely (if ever)
(1) send signals to the client or
(2) perform blocking operations based on their local data;
the reason for this is that,
(1) if every server process sent a signal when an action was performed, a large
    number of servers would quickly overwhelm the client and
(2) since each server process holds a different portion of the data,
    not every server process would block at the same time which can lead to deadlocks
    and race conditions.
So, although there are infrequent exceptions,
be aware that ParaView takes great care to initiate all actions on the client,
even when it seems that the server should do so.

Integration with SMTK
---------------------

In ParaView, there are 3 types of objects that SMTK frequently interacts with:

* Subclasses of vtkObject which live on each server process and are "wrapped"
  by C++ code that allows remote invocation of their methods across the
  client-server (CS) connection each server maintains with either the rank-0
  server or the client.
  Several SMTK classes have instances owned by a "wrapper" class that inherits
  from vtkObject to make this communication possible.
* Subclasses of vtkSMProxy, which live on the client process.
  Subclasses specific to SMTK are only required when the client needs to expose
  methods to manage SMTK data on the server that cannot be handled by
  ParaView's client-server wrapping. For example, methods that take non-primitive
  objects, such as pointers to SMTK classes as input, since these methods
  cannot be CS-wrapped.
* Subclasses of pqProxy, which live on the client process and reference
  an instance of vtkSMKProxy (or its subclasses).
  Subclasses specific to SMTK exist to expose Qt signals and slots related to
  SMTK. If Qt is not required, then you should subclass vtkSMProxy instead.
