Breaking changes to graph-resource arcs
---------------------------------------

The way SMTK represents arcs has changed.
This is a breaking change to accommodate new functionality:
the ability to provide or request indexing of both the
"forward" arc direction (from, to) and the "reverse" arc
direction (to, from).

Previously, the :smtk:`ArcMap <smtk::graph::ArcMap>` class held arcs
in a :smtk:`TypeMap <smtk::graph::TypeMap>` (a map of maps from
arc type-name to UUID to arc API object).
Now the ArcMap is a :smtk:`TypeContainer <smtk::graph::TypeContainer>`
(a simple map from arc ``typeid`` hash code to arc API object).

The arc classes have also changed.
If you previously did not inherit any of SMTK's arc classes,
you will need to adapt your own arc classes to expose new
methods. See the graph-session documentation for the new
method(s) you must provide.
If you previously inherited Arc, Arcs, or OrderedArcs in your
resource, these classes are removed.
Instead, if you do not provide implementations of methods for
accessing and manipulating arcs, the
:smtk:`implementation <smtk::graph::ArcImplementation>` will provide
them for you by applying the
:smtk:`ExplicitArcs <smtk::graph::ExplicitArcs>` template to
your traits class.

For more details, see the updated documentation for the
graph subsystem.
