Key Concepts
============

The graph of tasks (with dependencies as arcs) indicates what tasks a user may
work on, what tasks are incomplete, and what tasks cannot be performed because of
unmet dependencies.

A task becomes active when its dependencies are met and the user
chooses to focus on the task.
An application can compute the set of tasks which users
are allowed to focus on (make active) by cutting the graph along arcs
that connect completed tasks to incomplete tasks.

:smtk:`State <smtk::task::State>`
  is an enumeration of the states a task may take on.
  Tasks are unavailable until dependencies are met; then they are
  incomplete until the task's condition is met; then they are
  completable (but not completed) until the user marks the task
  as complete.

:smtk:`Task <smtk::task::Task>`
  instances have dependent tasks and a flag to store whether the user has
  marked a given task as complete.
  You may observe task state transitions.
  You should subclass this to implement logic that determines whether
  your task is unavailable, incomplete, or completable; the base class
  is unavailable until its dependencies are met, at which time it
  will transition straight to completable.

:smtk:`Instances <smtk::task::Instances>`
  is used to create instances of registered task classes.
  Any plugins that provide new task subclasses should
  register those classes with the factory in their registrar
  (see :ref:`smtk-plugin-sys`).

:smtk:`Active <smtk::task::Active>`
  is used to get the currently active task or switch to a different task.
  There can only be one active task at a time, although there may be
  no active task.
  Active tasks must be managed instances so that there is some
  indication before deletion that the task will be destroyed and
  thus should not be active.
  This object can be observed for changes to the active task.

:smtk:`Manager <smtk::task::Manager>`
  is an object applications can create to hold a task factory and
  the set of task instances the factory has created.
  It also holds the active task tracker.
