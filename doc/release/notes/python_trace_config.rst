Operation python tracing with config
------------------------------------

Python traces of operations now use a python dictionary to configure
the inputs to the operation. This is much easier to read and edit
than the complete XML used previously, and allows shorthand for
simple items.

Group items are not yet supported, and nested items must currently
be retrieved with their "path" instead of their "name".
