Project subsystem
-----------------

Project manager changes
~~~~~~~~~~~~~~~~~~~~~~~

The project manager no longer automatically manages projects as they are created.

Instead, the project manager observes operations which create projects and manage
any new projects upon completion. This matches the pattern set by the resource
manager and avoids observers being fired during operations when the project may
not be in a valid state.

If your code explicitly calls ``smtk::project::Manager::create(typeName)`` outside
of an operation, you now need to explicitly ``add()`` the project to the manager.
If you call ``create(typeName)`` inside an operation, you must be sure to add the
project to your operation's Result (in a ReferenceItem) so it can be added.
If you call ``smtk::project::Manager::remove(project)`` inside an operation, you
should not do so any longer. Instead, you should add the project to the
operation-result's ``resourcesToExpunge`` ReferenceItem. The base Operation class
will remove the project from its manager after the operation's observers have been
invoked to properly order Operation and Resource observers before Project observers.
