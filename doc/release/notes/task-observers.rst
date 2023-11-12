Task system
-----------

All task observers have changed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The task-instance, adaptor-instance, and workflow-event observers have all
changed to accommodate the fact that tasks and adaptors now inherit
:smtk:`smtk::resource::Component` and thus should only be modified inside
operations.

Because tasks and adaptors are modified inside operations (which run on
threads, not in the GUI thread), any observers to task/adaptor/workflow
events (creation/destruction/modification) must not be invoked immediately.

+ Instead of using the observers provided by :smtk:`smtk::task::Instances`
  and :smtk:`smtk::task::adaptor::Instances`, use the observers returned
  by the task-manager's ``taskObservers()`` and ``adaptorObservers()``
  methods, respectively.
+ Instead of using the workflow observer formerly provided by
  :smtk:`smtk::task::Instances` (which has been removed), use the
  task-manager's ``workflowObservers()`` method.

All of the observers provided by the task manager are initiated
by observing operations; the task-manager observes the operation manager
with a priority of ``operationObserverPriority()`` and invokes the observers
above as needed after each operation.

All of the Qt thread-forwarding for the previous observers has been
removed since operation observers (and thus the task-manager's task-related
observers) already run on the GUI thread.
