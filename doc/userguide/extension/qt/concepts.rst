Key Concepts
------------

One of the primary goals in the design of SMTK's Qt subsystem
was to allow operations to run in the background while keeping
the user interface responsive.
The Qt library supports threads, but requires any code that
modifies widgets to run on the "GUI thread" (i.e., thread 0,
the main application thread).

Designing thread support into a library is difficult, and
Qt's decision to force user-interface code to run on a specific
thread simplifies its design.
Similarly, SMTK makes some assumptions in order to simplify
support for simultaneous :smtk:`smtk::operation::Operation`
instances running in the background:

1. Locking occurs with a granularity of :smtk:`smtk::resource::Resource`
   instances, not components or other objects.
2. Locks on resources must **never** be acquired on the GUI thread.
3. Operation observers must always be invoked on the GUI thread
   while the locks the the operation's resources are held by the
   thread on which the operation is running.
4. Operations should be run via a :smtk:`smtk::operation::Launcher`
   so that they run asynchronously in the background as the
   operation's thread is able to acquire the locks it needs.

To ensure that operation observers are invoked on the GUI thread,
your application must create and use an instance of the
:smtk:`qtSMTKCallObserversOnMainThreadBehavior` class.
This class overrides the operation observer-invoker with a Qt-object
that uses Qt's signal-and-slot functionality to transfer execution
of observers to the GUI thread.
