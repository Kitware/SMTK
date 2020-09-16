Observers
=========

SMTK's observers pattern (:smtk:`Observers <smtk::common::Observers>`)
is a generic pattern for registering and executing callback routines
when an "observed" class performs different tasks. The observed class
traditionally contains an instance of the Observers class, which is
used to both register and call observer functors.

An observer functor is registered to an Observers instance with an
optional integral priority value (higher priority functors are called
before lower priority functors), an option to execute the observer
functor immediately upon registration, and a description that is
printed in debug mode when the observer functor is called. When an
observer functor is registered, a non-copyable key is returned that
scopes the lifetime of the observer functor (when the key goes out of
scope, the observer functor is unregistered from the Observers
instance).

If the Observers instance is constructed to have observers return a
boolean value, observation is terminated if an observer functor
returns false. This allows observers to potentially cancel an action
that is being observed, letting the observers drive the actions of the
observed class. If the Observers instance is constructed to have
observers return void, all observer functors will be called without
the possibility of cancellation.

The observers pattern also contains logic to allow consumers to
optionally modify the execution logic of the observer instance. This
feature is used to control the context in which observer functors are
executed (e.g., forcing all observer functors to execute on the
applicaiton's main thread).

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/comon/testing/cxx/UnitTestObservers.cxx`.
