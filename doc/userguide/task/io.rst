.. _smtk-task-io:

Serialization and deserialization
=================================

Tasks are created by passing JSON configuration data;
this constitutes the majority of deserialization.
However, beyond configuration, deserialization involves
connecting tasks via dependencies and potentially other
information – such as functors and references to SMTK's
various managers – that cannot be simply encoded into JSON.

For this reason, the task manager may be serialized and
deserialized. It uses a :smtk:`helper <smtk::task::json::Helper>`
to swizzle_ pointers to tasks
into integer indices during serialization and adds a phase
during deserialization that un-swizzles the integers back
into pointers to the objects it created.

Serialization also requires a functor for each class that
examines a task and returns a JSON object with its
full serialization.
See :smtk:`smtk::task::json::jsonTask` for an example
and note that subclasses of task should invoke the
functor for their superclass rather than trying to
reproduce its logic.

.. _swizzle: https://en.wikipedia.org/wiki/Pointer_swizzling
