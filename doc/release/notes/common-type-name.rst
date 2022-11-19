Type-name now reported consistently across platforms
----------------------------------------------------

The :smtk:`smtk::common::typeName` templated-function now returns
a string that is consistent across platforms. Previously, MSVC
compilers would prepend "``class ``" (already fixed) or "``struct ``"
(newly fixed), unlike other platforms. They also report anonmyous
namespaces differently (``\`anonymous namespace'`` vs ``(anonymous namespace)``.
These are now adjusted so that type names are consistent.
This is required for python bindings for the graph-resource, which
use the reported type names of arcs (which may be structs and may
live in anonymous namespaces) when editing the resource.

If you previously had code that special-cased MSVC type names, you
should remove it.
