Observers
=========

SMTK's observers pattern (:smtk:`Observers <smtk::common::Observers>`)
is a generic pattern for registering and executing callback routines
when an "observed" class performs different tasks related to events
in its lifetime or notifying others of its state.
The observed class traditionally contains an instance of the Observers class,
which is used to both register and call observer functors as these
events occur.

Observer parameters
-------------------

When an event that should be observed occurs, the Observers
class may be provided with information about the event;
this information determines the signature of the observer functors
and is a template parameter to the observer class.
For example, SMTK's resource manager considers the addition,
modification, and removal of resources to be significant events.
It passes an enum specifying which of these events is occurring
along with a shared pointer to the resource in question.

Observing state
---------------

Sometimes, observers may wish to observe the state of a class rather
than lifecycle events;
for example, SMTK's resource manager notifies observers when resources
are added to or removed from the manager. However, observers may need
to maintain a list of all the resources in the manager — even those
added before the observer was registered.
Thus when an observer functor is registered with Observers, there is
sometimes a need for the functor to be called immediately with the
initial state as well as later when future events alter the state.

If this is the case, the Observers instance can be provided with a
function at construction that "initializes" observers upon registration
with the current state. If an initializer is provided, observers can opt in
to being initialized as they are registered; if no initializer is
provided, then any request to be initialized is ignored.

Registering an observer
-----------------------

An observer functor is registered to an Observers instance with

+ an optional integral priority value (higher priority functors
  are called before lower priority functors),
+ an option to initialize the observer by executing the functor
  immediately upon registration, and
+ a string description that is printed in debug mode when the
  observer functor is called.

When an observer functor is registered, a non-copyable key is returned that
scopes the lifetime of the observer functor (when the key goes out of
scope, the observer functor is unregistered from the Observers
instance).

Early exit
----------

If the Observers instance is constructed to have observers return a
boolean value, observation is terminated if an observer functor
returns false. This allows observers to potentially cancel an action
that is being observed, letting the observers drive the actions of the
observed class. If the Observers instance is constructed to have
observers return void, all observer functors will be called without
the possibility of cancellation.

Mutating observations and thread safety
---------------------------------------

Note that observers are normally invoked synchronously whenever
the parameters of the observed event are provided to the Observers'
parenthesis operator.
If used in a threaded environment, this means that multiple threads
might potentially invoke observers — potentially simultaneously.
If this is not your intention (many user interface libraries do not
allow updates except on one thread), you must modify how observers
are called.

The Observers pattern also contains logic to
optionally modify the execution logic of the observer instance. This
feature is used to control the context in which observer functors are
executed (e.g., forcing all observer functors to execute on the
applicaiton's main thread, as is done by
:smtk:`pqSMTKCallObserversOnMainThreadBehavior`).

Example
-------

An example that demonstrates the prinicples and API of this pattern
can be found in SMTK's UnitTestObservers_ test.

.. _UnitTestObservers: https://gitlab.kitware.com/cmb/smtk/-/blob/master/smtk/common/testing/cxx/UnitTestObservers.cxx
