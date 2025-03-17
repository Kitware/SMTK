Task System
===========

Active task notifications
-------------------------

Observers of the :smtk:`smtk::task::Active` are now invoked after
(rather than before) the active task has been changed.
This means that responders can fetch the active task inside the
observer and it will match the "next" task rather than the current task.
