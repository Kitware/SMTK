Debugging observers
===================

Observers are documented in the :ref:`observers-pattern` section of the manual.
The issues to be aware of when debugging observers are:

* Most observers inherit :smtk:`smtk::common::Observers`, which is templated on
  the signature of functions to be called when an event occurs.
* The :smtk:`Observers <smtk::common::Observers>` class has a second template
  parameter (a boolean) that turns on debug printouts when true.
  The default is false.
* Observers are forced to run on the main (GUI) thread in ParaView
  by the :smtk:`pqSMTKCallObserversOnMainThreadBehavior` class.
  This means that observers are called asynchronously at some point after the
  operation has completed and potentially after other SMTK calls have been made.
  Because operation results *may* hold shared pointers to components and resources
  (thus keeping them alive), destructors may not be called when you expect.
* An observer may remove itself from an Observers instance while it is being
  evaluated. The actual erasure will occur after all observers for the event
  have been invoked. When this occurs, it is noted in the debug printouts.

What the above means is that if you are having issues with observers of,
for example, resource manager events, you will want to change `smtk/resource/Observer.h`
from

.. code:: c++

   typedef smtk::common::Observers<Observer> Observers

to

.. code:: c++

   typedef smtk::common::Observers<Observer, /* debug */ true> Observers

and rebuild+install SMTK. Then, information about the lifecycle of each observer
in the resource-manager event will be reported (its addition, invocation, and removal).
