Task subsystem changes
----------------------

The task manager is no longer considered part of an application's state.
Instead of expecting an application's :smtk:`Managers <smtk::common::Managers>` instance to
hold a single task manager, each :smtk:`Project <smtk::project::Project>` owns its own
task manager.

As part of this change, the project read and write operations now include a serialization of
the project's task manager. This means that the task system JSON state is now properly
serialized.

Another part of this change removes the :smtk:`smtk::task::json::jsonManager` structure
with its ``serialize()`` and ``deserialize()`` methods. You should replace calls to
these methods with calls to ``to_json()`` and ``from_json()``, respectively.
Furthermore, you are responsible for pushing an instance of the
:smtk:`task helper <smtk::task::json::Helper>` before these calls and popping the instance
afterward.
See the task tests for examples of this.

Finally, because :smtk:`smtk::common::Managers` no longer contains an application-wide
instance of a :smtk:`smtk::task::Manager`, the signature for :smtk:`Task <smtk::task::Task>`
constructors is changed to additionally accept a parent task manager.
The old signatures will generate compile- and run-time warnings.
The constructors still accept a :smtk:`smtk::common::Managers` since tasks may wish
to monitor the application to determine their state.
