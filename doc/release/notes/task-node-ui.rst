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

Task-node modifications
~~~~~~~~~~~~~~~~~~~~~~~

Now :smtk:`smtk::extension::qtBaseTaskNode` has a ``updateToMatchModifiedTask()``
method called whenever an operation modifies a task. It is up to the node to
ensure its visual representation (but not its incoming/outgoing arcs) are
up-to-date with the modified task. Usually, this is just ensuring the label
matches the task's name/title.

Task-editor interaction modes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :smtk:`smtk::extension::qtTaskEditor` has been refactored so that each
user-interaction mode is a separate class that installs event filters on
the :smtk:`smtk::extension::qtTaskView` and adds a QAction to the task-panel's
toolbar. The QAction is checkable and, when triggered, causes the editor to
enter its corresponding interaction mode.
All the modes' actions belong to a ``QActionGroup`` so that only one action
may be selected at a time.

The task editor now provides 4 modes: one for panning the viewport,
one for selecting task nodes, one for connecting nodes via arcs, and
one for removing arcs between nodes.

Arc editing
~~~~~~~~~~~

As discussed above, arcs may be created and removed via the task editor.
The task editor's :smtk:`smtk::extension::qtConnectMode` monitors the
:smtk:`smtk::operation::ArcCreator` operation group to discover what types
of operations exist to create arcs (and what type(s) of arcs each operation
can create) and launches the user-indicated operation to connect a selected
pair of nodes.

Arcs may be removed by entering the :smtk:`smtk::extension::qtDisconnectMode`
and clicking the backspace key with one or more arcs selected.
If no operation exists to delete a selected arc, no action is taken except
that an error is reported.
