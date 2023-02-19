Operation hint for switching the active task
--------------------------------------------

Any operation can now request the active task be switched
by providing a hint on its result attribute.
Use the ``smtk::operation::addActivateTaskHint()`` function
to add the hint before your operation completes.
Then, the :smtk:`pqSMTKOperationHintsBehavior` object will
observe when the operation has completed and process the
hint and attempt to switch to the matching task.
See ``smtk::project::Read`` for an example.
