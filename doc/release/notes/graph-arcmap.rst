Graph-resource ArcMap
=====================

The graph resource now provides a custom subclass of TypeMap named smtk::graph::ArcMap.
This subclass deletes the copy and assignment constructors to prevent modifications
to accidental copies of arcs rather than to the resource's arc container.

External changes
----------------

This change removes the smtk::graph::ResourceBase::ArcSet type-alias; instead
the ArcMap class is used directly.

If you maintain external code that depends on smtk::graph::Resource, you will
need to replace the ResourceBase::ArcSet type-alias with ArcMap (which lives
in smtk::graph not smtk::graph::ResourceBase).
