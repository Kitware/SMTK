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

Job-runner artifact controls no longer change ParaView representation
visibility from their destructors. Normal job and view transitions continue
to hide displayed artifacts explicitly. Active-task transitions also hide the
previous task's artifacts while the rendering UI is still valid, while
application shutdown deletes the controls without emitting
active-representation events to rendering UI observers that may already be
partially destroyed.

The attribute panel now guards queued active-pipeline-source updates with a
``QPointer``. If a source is removed before Qt processes the update, as can
happen when leaving post-processing mode or switching projects, the pending
display request is discarded instead of passing a dangling pipeline-source
pointer to the panel.

Together, these changes prevent crashes observed when switching projects or
quitting an SMTK-based application after displaying task, attribute, or job
result views.
