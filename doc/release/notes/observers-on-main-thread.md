## Add logic in the PV layer to force observers to fire on main thread

Qt requires that all methods that affect the GUI be performed on the application's main thread. Many of the registered Observer functions for both operations and resources are designed to affect the GUI. Rather than connect GUI-modifying logic to a signal triggered by an observer, we mutate the behavior of the operation and resource Observer calling logic to ensure that all Observer functors are called on the main thread, regardless of which thread performs the observation.

To support this pattern, SMTK's Observer pattern has been generalized to a single description (smtk::common::Observers) that adopts a run-time configurable type of polymorphism where consuming code can change the class's behavior, allowing consuming code to redefine the context in which the Observer functors are executed.
