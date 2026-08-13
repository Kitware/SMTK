Resource and Project Lifecycle
==============================

Project switching and application shutdown
------------------------------------------

SMTK now releases project-owned task managers when their project is closed,
rather than allowing plugin registries to retain them until application or
shared-library shutdown. Plugin registrations hold weak references to their
managers and unregister from managers that are still alive. ParaView's SMTK
wrapper also explicitly unregisters plugins from its application-scoped
manager collection during teardown, in dependency order, while SMTK's
process-wide services remain available.

Reference items no longer resolve expired resource surrogates when they are
detached from their owning resource. Values already present in the reference
item's cache are preserved, but detachment does not perform I/O or reload a
resource that an application has just closed. This prevents operation and task
destruction from reopening project resources during teardown.

Task-path tool buttons now hold weak references to tasks. Since these widgets
may be deleted asynchronously by Qt, they unregister their task observer only
when the task is still alive. Thus, deferred widget deletion neither retains a
closed project's task hierarchy nor accesses a task after its project-owned
task manager has been destroyed.

Together, these changes prevent crashes observed when switching projects or
quitting an SMTK-based application after displaying task and attribute views.
