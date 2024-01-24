Key Concepts
============

Where the geometric model resource has a fixed set of component types,
the graph-based model is intended to hold application-specific components.
Furthermore, it uses modern C++ template metaprogramming techniques to
offer type safety.

:smtk:`Resource <smtk::graph::Resource>`
  instances contain graph nodes and arcs (which are traditionally called
  graph edges — but we wish to avoid confusion with geometric edges).
  All of the nodes you create are owned by the resource, along
  with the arcs between them.
  By default, nodes are held in a :smtk:`NodeSet <smtk::graph::NodeSet>`
  whose underlying storage is a ``std::set<std::shared_ptr<smtk::graph::Component>>``
  with a comparator that orders components by UUID.
  However alternate types of storage are supported and the
  programming interface is geared to make indexing nodes by their
  UUID, type, and name possible.

  The resource class itself requires a single template parameter
  that is a type-traits object which
  contains

  + a ``NodeTypes`` type-alias, expected to be a ``std::tuple``
    of subclasses of the graph component class described below;
  + an ``ArcTypes`` type-alias, expected to be a ``std::tuple``
    of arcs allowed to connect nodes to form a graph; and
  + an optional ``NodeContainer`` type-alias which the resource
    will inherit as the container for shared-pointers to nodes
    (instead of :smtk:`NodeSet <smtk::graph::NodeSet>`) if it
    is present.

  Thus the resource class is not a concrete implementation but
  rather a base class for other resources.

:smtk:`Component <smtk::graph::Component>`
  is a subclass of the geometry subsystem's
  :smtk:`Component <smtk::geometry::Component>`;
  instances of graph components serve as nodes in the resource's graph.
  This subclass exists to extend the base API with methods for
  accessing graph nodes related by registered arc types.
  As above, you are expected to subclass this class with node types
  specific to your application.

Arc
  is any struct or class holding type-traits and methods that specify
  how arcs of the given type behave (i.e., what types of nodes they
  may connect) and, optionally, are stored.
  An arc class may present either *explicit* or *implicit* arcs.

  Explicit arcs are those which are explicitly stored by SMTK as pairs
  of connected node IDs. Any arc whose struct/class does **not**
  provide visitor methods (discussed below) is considered explicit.

  Implicit arcs are those which are implied by the nature of data being
  represented (e.g., in a structured grid, points are implicitly connected
  to neighors; also, every node in one layer of a neural network is
  connected to every node in the next layer – given a pair of nodes in
  the neural network it is easy to decide whether they are connected
  by looking at their layer assignment) or by a third-party library
  (e.g., a CAD modeling kernel already stores relationships between
  edges and faces; rather than duplicate and maintain this relationship
  in SMTK, we forward requests about connectivity to the CAD library).

  Resources that include an arc class in their list of arc types
  *decorate* the type-traits class you pass with templates that
  completely implement a convenient and consistent interface from
  the description and any partial implementation your arc class provides.

  The decorated version of your arc class is then instantiated and
  stored as an entry in the resource's :smtk:`ArcMap <smtk::graph::ArcMap>`.
  The arc map, held by a resource instance, is where explicit arc
  storage lives (if any).

Developers who wish to construct their own implementation of a graph
resource should start by going through
`smtk/graph/testing/cxx/TestArcs.cxx` to understand the
front-facing API before going through the implementation.

.. todo::

   Create runtime-configurable classes (ProgrammableResource,
   ProgrammableNode, ProgrammableArc) that can be Python-wrapped and
   configured via a JSON serialization.
