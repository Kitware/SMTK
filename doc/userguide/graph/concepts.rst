Key Concepts
============

Where the geometric model resource has a fixed set of component types,
the graph-based model is intended to hold application-specific components.
Furthermore, it uses modern C++ template metaprogramming techniques to
offer type safety.

:smtk:`Resource <smtk::graph::Resource>`
  instances contain graph nodes and arcs (which are traditionally called
  graph edges — but we wish to avoid confusion with geometric edges).
  All of the model entities you create are owned by the resource, along
  with the arcs between them. The resource class itself is templated.
  It requires a type-traits object as its template parameter that list
  the node and arc types the resource will accept. Thus it is not
  a concrete implementation but rather a base class for other resources.

:smtk:`Component <smtk::graph::Component>`
  is a subclass of :smtk:`Component <smtk::geometry::Component>` that
  provides methods for accessing nodes connected to it via arcs. As
  above, you are expected to subclass this class with node types
  specific to your application.

Arc
  describes a directed arc (or collection of arcs) from one node to
  another node, and is typically subclassed to differentiate between
  different arc types within a multipartite graph. Developers can
  construct their own arc types to suit their needs; three commonly
  used arc type templates are provided. :smtk:`Arc <smtk::graph::Arc>`
  describes an arc from one node to another, :smtk:`Arcs
  <smtk::graph::Arcs>` describes a set of arcs from one node to
  another node, and :smtk:`OrderedArcs <smtk::graph::OrderedArcs>`
  describes an ordered set of arcs from one node to another node.

Developers who wish to construct their own implementation of a graph
resource should start by going through
`smtk/graph/testing/cxx/TestPlanarResource.cxx` to understand the
front-facing API before going through the implementation.

.. todo::

   Create runtime-configurable classes (ProgrammableResource,
   ProgrammableNode, ProgrammableArc) that can be Python-wrapped and
   configured via a JSON serialization.
