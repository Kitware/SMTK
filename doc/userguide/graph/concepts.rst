Key Concepts
============

Where the geometric model resource has a fixed set of component types,
the graph-based model is intended to hold application-specific components.
Furthermore, it uses modern C++ template metaprogramming techniques to
offer type safety as well as programmable run-time component (graph node)
and arc classes that can be configured from Python scripts or even text
files by applications or end users.

:smtk:`Resource <smtk::graph::Resource>`
  instances contain graph nodes and arcs (which are traditionally called
  graph edges — but we wish to avoid confusion with geometric edges).
  All of the model entities you create are owned by the resource, along
  with the arcs between them. Arcs are represented via a mechanism similar
  to the resource :smtk:`Links <smtk::resource::Links>` API.
  The resource class itself is templated.
  It requires a type-traits object as its template parameter that list
  the node and arc types the resource will accept. Thus it is not
  a concrete implementation but rather a base class for other resources.

:smtk:`Component <smtk::graph::Component>`
  is a subclass of :smtk:`Component <smtk::geometry::Component>` that provides
  some methods for accessing connected arcs.
  As above, you are expected to subclass this class with node types
  specific to your application.

.. todo::

   Create runtime-configurable classes (ProgrammableResource, ProgrammableNode,
   ProgrammableArc) that can be python-wrapped and configured via a JSON serialization.

Nodes and Arcs
==============

This section covers how to subclass SMTK's graph entities to fit your application's model.

Nodes
-----

Nodes are intended to be lightweight subclasses of :smtk:`Component <smtk::graph::Component>`.
Where possible, you should consider using SMTK's property system instead of adding member data
to a graph node; properties are serialized when reading and writing resources and the graph-based
model resource provides queries to find components based on properties.
By using properties for storage, you can take advantage of this work.
Think of graph node classes as repositories of methods that
query and modify the component's properties.

.. todo:: Describe minimal subclass

Arcs
----

Similarly to graph nodes, think of arcs as repositories of methods that act on
relationships between components.
Arcs are directed graph-edges that restrict the two components at either endpoint
to be of a specific type.
For instance, a geometric face can be bounded by 0 or more edges.
The arcs that connect a face to its bounding edges only accept a Face component
for their origin and an Edge component for their destination.

The graph-model resource provides two templated arc classes that your arcs may inherit:
Arcs and OrderedArcs.
The latter can be used to preserve the ordering of arcs relative to one another.
This is useful in solid modeling (CAD) where order is related to geometric arrangement.
For example, the edges bounding a face can be ordered into loops whose edges meet
head-to-tail and always keep the face interior to the left when traversing edges in order.

.. todo:: Describe use of struct

Filtering and Searching
=======================

.. todo:: Describe filter/search syntax
