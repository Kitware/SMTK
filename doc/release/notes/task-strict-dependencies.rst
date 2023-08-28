Task system
===========

Strict dependency processing
----------------------------

The base :smtk:`smtk::task::Task` class now has an additional configuration
parameter indicating whether its dependencies should be strictly enforced
when computing its state or not.
You can call the ``areDependenciesStrict()`` method to see if it is set
or not (the default is false).

There is not currently a method to change whether dependencies are strictly
enforced or not since it is not intended to be changed during a workflow
but rather set at the time the workflow is designed. This may change but
would require significant work.

When strict dependency checking is enabled, its state will be Unavailable
until all of its dependencies are marked Completed by a user (not just
when they are Completable).
