New task classes and base class changes
=======================================

The task subsystem now provides more task types, task-adaptor classes
for configuring tasks as they change state, and additional tests.
See the `task class documentation`_ for details.

Tasks now include "style" strings that will be used to configure
application state when the task becomes active.

Tasks now include references to dependencies and dependents,
children and a parent. These are used to provide workflow
observers that user interfaces can use to monitor when tasks
are added-to and removed-from a pipeline.

.. _task class documentation: https://smtk.readthedocs.io/en/latest/userguide/task/classes.html
