Behaviors
---------

In ParaView, _behaviors_ are plugin objects that monitor the
state of objects and update the application user interface
to reflect the state of those objects.
For example, :smtk:`pqSMTKBehavior` monitors client/server
connections and ensures core SMTK plugins are loaded on each
new server connection.

Behaviors are typically implemented as auto-start plugin
code that uses ParaView's signal/slot mechanism to identify
events of interest, create or modify user interface elements
such as menus, panels, toolbars, etc.

Hint Behavior
~~~~~~~~~~~~~

The :smtk:`pqSMTKOperationHintsBehavior` monitors the application's
operation-manager; when operations complete, it inspects the result
of the operation and searches for hint attributes. When it finds
them, it implements the requested UI functionality.
For example, when a hint suggesting some resources and/or their
components should be hidden (or shown), this class updates the
ParaView representation visibility and/or per-block visibility.

Resource Visibility Behavior
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :smtk:`pqSMTKTaskResourceVisibility` monitors project-manager events.
When a project is added, it monitors the project's task-manager for
changes to the active task.
As the active task is modified, this plugin searches for any style
tags called out by the active task; if the style associated with these
tags includes a ``3d-view`` directive, the visibility and/or color-by
modes are changed.

Note that when the active task becomes null (i.e., a task is made inactive but
no new task is made active in its place), this behavior examines the
diagram panel to see if top-level tasks are being displayed or not.
If top-level tasks are being displayed, the behavior will apply the ``default``
style tag if any exists; otherwise, the style of the parent task whose children
are being displayed will be applied.
