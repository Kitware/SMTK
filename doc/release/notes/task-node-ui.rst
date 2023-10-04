Changes to Task UI Architecture
-------------------------------

Task-node state consistency
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :smtk:`smtk::extension::qtBaseTaskNode`'s ``updateTaskState()`` method
now takes an additional parameter indicating whether the task is active or not.
This prevents an inconsistency between the UI and the task manager because
this function is called during transitions between active tasks;
since it was called for both nodes during the transition, ``isActive()``
would only return the proper result for one task.

Now, ``updateTaskState()`` is invoked by the :smtk:`smtk::extension::qtTaskEditor`
as it observes active-task transitions.

Finally, the task node initiates transitions in the active task (as the
user clicks on the title bar) but waits for a call from the editor to
update its user interface.
