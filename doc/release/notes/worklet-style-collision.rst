Task Subsystem
==============

Worklet style collisions
------------------------

When a worklet is emplaced, it is deserialized as if it were a task manager unto itself.
However, this can cause an issue when the project's task manager already contains
definitions for style tags with the same name (for example, if two worklets in the gallery
define a "show-geometry" style with different values). Previously, each worklet emplaced
would overwrite the old style definition with its own (breaking already-emplaced tasks).
Now, if name collisions between style tags already present in the task manager are detected
and the style definitions are not identical, a new style tag is created and used.
