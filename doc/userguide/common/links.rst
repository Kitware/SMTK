Links
=====

SMTK's links pattern (:smtk:`Links <smtk::common::Links>`) describes a
set-like container of "link" objects that have a user-defined base
class and represent a connection between two different object types (a
"left" type and a "right" type). Additionally, each link contains a
"role" field to contextualize the link. The pattern uses boost's
multi-index array to facilitate efficient access to a subset of links
according to their "left", "right" and "role" values.

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/comon/testing/cxx/UnitTestLinks.cxx`.

Note that the links template is used by SMTK's resource system in a
two-level hierarchy: one Links instance connecting resources to each
other with a "generic" role holds a child Links instance that connects
both components and resources in specific, user-provided roles.
See :ref:`resource links` for more information.
