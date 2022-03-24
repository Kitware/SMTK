Nodes and Arcs
==============

The graph session has been designed to accommodate a broad range of
use cases, including situations where the graph's arc data exist in an
external modeling kernel and are "discovered" as they are
accessed. This level of flexibility is achieved by using generic
programming principles to describe the graph's node and arc types and
API, while recovering compile-time type safety using the graph
resource's traits class that lists its available nodes and arcs. The
relationship between nodes and arcs is modeled using a generic
accessor pattern, where the nodes are the first-class elements of the
graph and the arcs augment the nodes' API to access neighboring nodes.

To declare the node and arc types a resource accepts,
define a graph-traits class with type-aliases named ``NodeTypes``
and ``ArcTypes`` that are ``std::tuple`` objects listing
types for nodes (which must inherit :smtk:`smtk::graph::Component`)
and arcs (which must provide type-aliases described below),
respectively.

.. literalinclude:: ../../../smtk/graph/testing/cxx/TestArcs.h
   :language: c++
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --
   :linenos:

Although not present in the example above,
you may also provide a type-alias named ``NodeContainer``
that names the type of container the resource should use to hold
references to its nodes.
If present, the ``NodeContainer`` class will be publicly inherited
by your resource, making its storage available as you see fit.
By default, :smtk:`smtk::graph::NodeSet` is used, which is simply a
set of shared-pointers to components.
You may wish to consider the boost multi-index container as an
alternative, since it may be indexed by node type (a common query).
In that case, see the NodeSet API required for custom storage.

The type-traits class, such as the ``ExampleTraits`` class above, is
used by several templated classes to compose implementations of
methods used to access nodes and arcs in resources and components
as shown below. We will cover these in more detail in the following
sections.

.. findfigure:: graph-arcs-decoration.*
   :align: center
   :width: 95%

   How graph-traits type-aliases are processed with templates into
   actual node and arc implementations.
   In most cases, you will not need to implement storage for arcs.

Nodes
-----

As the first-class element of the graph structure, :smtk:`Component
<smtk::graph::Component>` (and types that derive from it) have access
to the parent resource and node id. The remainder of its API
(``outgoing()``, ``incoming()``, ``set()``, and ``get()``) exists to
provide access to other nodes related by arc types as described below.

Graph components (nodes) do not store arcs themselves – they expose
arcs from their parent resource.
This separation of API implementation from the node to the arc
facilitates the traversal of arcs or sets of arcs with different storage
layouts without modifying the node class description.

Note that, unlike arcs – whose storage is segregated by type – nodes
must be globally indexed (by UUID at a minimum). Otherwise we would
use the same storage pattern that arcs use.
You can specify a type-alias, named ``Serializable`` on a node type
indicating you would like it to be serialized when saving the resource.
Otherwise, nodes are not serialized to file.

Arcs
----

As mentioned above, any class (or struct; we'll use class here for simplicity)
may describe a type of arc; inheriting a common base class is not required.
Instead, arc classes are used as type-traits objects that specify how
arcs behave via type-aliases and class-static methods.
Note that each type of arc is responsible for storing *all* arcs of that type
for the entire resource.
This table summarizes the type-aliases available (some of which are
mandatory):

.. list-table:: Type aliases for graph-arc objects. (☑ = mandatory; ☐ = optional)
   :widths: 10 5 35
   :header-rows: 1

   * - Type alias
     - ☐/☑
     - Description

   * - ``FromType``
     - ☑
     - The type of node serving as the source (origin) of the arc.

   * - ``ToType``
     - ☑
     - The type of node serving as the destination (end) of the arc.

   * - ``Directed``
     - ☑
     - True (``std::true_type``) or false (``std::false_type``) indicating
       whether the arcs are directed (true) or undirected (false).

   * - ``ForwardIndexOnly``
     - ☐
     - If present and true (``std::true_type``), it indicates
       that no method exists to discover connected nodes given a
       destination node (i.e., you may only visit, count, or test
       connections from outgoing (source) nodes).

       When this type-alias is absent, bidirectional indexing is
       presumed. In that case, you must provide an ``inVisitor``
       method as described below.

       This tag is unsupported and will cause an assertion at
       compile time if the arc type is undirected.

   * - ``Immutable``
     - ☐
     - If present and true, this mark forces the arc editing methods
       on the :smtk:`smtk::graph::ArcImplementation` to have no effect
       (and always return false), even if the traits class provides
       ``connect`` and ``disconnect`` methods.
       This is mainly intended for explicit arcs that should not allow
       user editing. (In this case, the resource might provide methods
       to use the underlying :smtk:`smtk::graph::ExplicitArcs` API in
       a way that enforces model consistency.)

   * - ``Ordered``
     - ☐
     - If present, this must be true (``std::true_type``), indicating
       that, when multiple arcs terminate at the same node, the order
       in which they are traversed is significant.

       This is used, for example, in CAD boundary-representation models
       to order the edges bounding a face loop.
       Assuming the arc's FromType is a ``Face`` and the ToType is an
       ``Edge``, then the order of edges reported by the outVisitor
       method (see below) is required to link the edges head-to-tail.
       Similarly, faces reported by the inVisitor method (see below),
       is frequently used to determine whether a given edge is oriented
       along the loop or opposing the loop direction.

       If present on explicit arcs, additional indexes are created in
       the arc container to preserve these orderings.

Arcs may be explicit (meaning the resource holds pairs of node UUIDs
representing each arc as part of its storage) or implicit (meaning that
arcs are generated procedurally).
An example use of implicit arcs is the OpenCascade session whose nodes
form the boundary-representation (B-Rep) of a CAD model.
The arcs are not stored by SMTK but instead are obtained by calling methods
on the OpenCascade modeling kernel.

Explicit arcs are the simplest to declare because SMTK provides
a default implementation for you (using :smtk:`smtk::graph::ExplicitArcs`).
For example:

.. literalinclude:: ../../../smtk/graph/testing/cxx/TestArcs.h
   :language: c++
   :start-after: ++ 2 ++
   :end-before: -- 2 --
   :linenos:

is all that's required.
SMTK will insert an instance of ExplicitArcs into the resource's ArcMap for you;
when users create or remove ``ExplicitArc`` arcs between nodes,
an instance of ``ExplicitArcs<ExplicitArc>`` in the resource's arc map is edited.
(NB: The ``ExplicitArcs<ExplicitArc>`` instance is not stored directly; we will
discuss that further in :ref:`smtk-graph-arc-implementation`).

Beyond the type aliases above, several class-static member variables
and methods may also be present in the arc-traits class if you wish
to avoid explicitly storing arcs in the resource.

.. list-table:: Constants and methods for graph-arc traits-type objects
   :widths: 25 25
   :header-rows: 1

   * - Member/method
     - Description

   * - ``std::size_t MaxOutDegree``
     - The maximum number of arcs of this type that any ``FromType`` node may have
       (i.e., the maximum number of outgoing arcs of this type).
       A value of smtk::graph::unconstrained() allows any number of attached nodes.
       This value is used as a hard constraint by the ExplicitArcs class.

   * - ``std::size_t MaxInDegree``
     - The maximum number of arcs of this type that any ``ToType`` node may have
       (i.e., the maximum number of incoming arcs of this type).
       A value of smtk::graph::unconstrained() allows any number of attached nodes.
       This value is used as a hard constraint by the ExplicitArcs class.

   * - ``std::size_t MinOutDegree``
     - The minimum number of arcs of this type that any ``FromType`` node may have
       (i.e., the minimum number of outgoing arcs of this type).
       A value of 0 (the default) means there is no constraint.
       This is not currently enforced or validated.

   * - ``std::size_t MinInDegree``
     - The minimum number of arcs of this type that any ``ToType`` node may have
       (i.e., the minimum number of incoming arcs of this type).
       A value of 0 (the default) means there is no constraint.
       This is not currently enforced or validated.

   * - ``template<typename Functor> smtk::common::Visited outVisitor(const FromType* node, Functor ff) const``
     - A method (usually templated on ``Functor`` type) that invokes the
       given functor with each destination (``ToType*``) connected to the given
       input node. The Functor is usually templated to allow (but not require)
       a return value to cause early termination.

       If this method is not provided, the arc is assumed to be explicit
       and an implementation will be provided for you.

   * - ``template<typename Functor> smtk::common::Visited inVisitor(const ToType* node, Functor ff) const``
     - A method (usually templated on ``Functor`` type) that invokes the
       given functor with each source (``FromType*``) connected to the given
       input node. The Functor is usually templated to allow (but not require)
       a return value to cause early termination.

       This method is only required on implicit arcs, and only if
       the ForwardIndexOnly trait is absent or false.

   * - ``template<typename Functor> smtk::common::Visited visitAllOutgoingNodes(Functor ff) const``
     - A method (usually templated on ``Functor`` type) that invokes the
       given functor on each node that has outgoing arcs.
       This, along with the ``outVisitor`` method above, can be used to
       iterate over all undirected arcs.
       When combined with visitAllIncomingNodes and inVisitor, then all directed
       arcs can be traversed efficiently.

       This method is optional and recommended only for implicit arcs.
       If you do not provide an implementation, then ArcImplementation
       will generate a slow version which traverses every node in the
       resource to identify those that may have outgoing arcs.

   * - ``template<typename Functor> smtk::common::Visited visitAllIncomingNodes(Functor ff) const``
     - A method (usually templated on ``Functor`` type) that invokes the
       given functor on each node that has incoming arcs.

       This method is optional and recommended only for implicit arcs.
       If you do not provide an implementation, then ArcImplementation
       will generate a slow version which traverses every node in the
       resource to identify those that may have incoming arcs.

   * - ``bool contains(const FromType* from, const ToType* to) const``
     - A method that returns true if an arc of this type exists between the
       given from and to nodes. For undirected arcs whose FromType and ToType
       are identical, this method must return the same value given either
       (from, to) or (to, from).

       If this method is not provided, an implementation that uses outVisitor
       will be provided (but this is to be avoided if possible since it will
       not be efficient).

   * - ``bool connect(const FromType* from, const ToType* to, const FromType* beforeFrom = nullptr, const ToType* beforeTo = nullptr)``
     - Insert an arc given its endpoint nodes and, optionally, a second pair
       of nodes specifying the the insertion location of each endpoint.
       The optional second set of (pre-existing) nodes is only used when
       the Ordered type-alias exists on this arc.

       If this method is not present on an implicit arc, insertion is impossible
       (i.e., the arcs are read-only).
       In this case, the decorated arc object will include an implementation
       of this method that always returns false so that users do not need
       to test for its existence.

   * - ``bool disconnect(const FromType* from, const ToType* to)``
     - Remove an arc given its endpoint nodes.

       If this method is not present on an implicit arc, removal is impossible.
       In this case, the decorated arc object will include an implementation
       of this method that always returns false so that users do not need
       to test for its existence.

A simple example of an arc out-node visitor from the ``TestArcs`` example is here:

.. literalinclude:: ../../../smtk/graph/testing/cxx/TestArcs.h
   :language: c++
   :start-after: // ++ 3 ++
   :end-before: // -- 3 --
   :linenos:

Note that the example above is contrived for the purpose of testing.

If only the ``outVisitor`` method is provided, then the resulting implicit
arc will only be traversible in the forward direction (i.e., given
a ``FromType`` node, visit its ``ToType`` nodes) and will be
immutable – meaning that no arcs may be added or removed by users.
(Generally, this means that the arcs are managed by a third party library
that only allows arcs to be added or removed in patterns consistent with
their modeling intent.)

Arc-trait properties
--------------------

SMTK provides an :smtk:`ArcProperties <smtk::graph::ArcProperties>`
template that can be used to examine traits objects.
It is used internally to decide how to decorate each arc's traits into
a class that provides a complete implementation (described in the next section)
of all the accessor and editor methods needed.


You can and should use this class when writing logic to analyze arcs.
It is important to understand that there are generally two different types
you might pass as template parameters to the arc properties class:

+ the user-provided arc traits class (in which case the returned properties
  describe what the user has passed to the
  :smtk:`ArcImplementation <smtk::graph::ArcImplementation>`); or
+ the computed arc-storage object (i.e., ``detail::SelectArcContainer<ArcTraits,ArcTraits>``),
  in which case the returned properties describe :smtk:`smtk::graph::ExplicitArcs`
  templated on the user-provided arc traits class.

Generally, you will want to pass the user-provided traits but
there are times you may wish to identify methods implemented by the arc-storage object.

.. _smtk-graph-arc-implementation:

Arc-trait decoration
--------------------

Since arc classes are not required to implement all of the methods described above,
SMTK provides the :smtk:`ArcImplementation <smtk::graph::ArcImplementation>` class
templated on your traits class.

The arc implementation template is responsible for

+ providing a complete and consistent set of methods for examining and
  editing arcs of one type;
+ holding an instance of some object (either your ``ArcTraits`` class or
  :smtk:`ExplicitArcs\<ArcTraits> <smtk::graph::ExplicitArcs>`) that stores
  or fetches arcs on demand; and
+ providing interface objects (see :ref:`smtk-graph-endpoint-interface`) for
  graph components to expose.

The storage that arc implementations use is selected by the
:smtk:`SelectArcContainer <smtk::graph::detail::SelectArcContainer>` template.
This template selects either the arc traits-class itself for storage
(in the case of implicit arcs where the traits-class implements methods as needed)
or one of SMTK's explicit arc-storage classes.

.. _smtk-graph-endpoint-interface:

Arc endpoints
-------------

You may have noticed that the arc-traits objects have an API that is functional
but not necessarily easy to use.
The arc implementation object also provides "endpoint interface" objects
(:smtk:`ArcEndpointInterface <smtk::graph::ArcEndpointInterface>`)
that behave as containers for all the nodes attached at one endpoint by
arcs of the given type.

The endpoint interface classes hold a reference to a single component
and provide access to nodes at the other end of arcs of the given type.
When the arc is mutable and the component is not const-referenced,
then methods are available to connect and disconnect nodes to the subject endpoint.

Arc direction and indexing
--------------------------

SMTK's implementation of an abstract graph involves adapting the
mathematical description of a graph to an implementation in the C++ language.
In order for the implementation to be consistent and efficient,
there are some places where the theoretical and practical implementations
differ.

+ Graph theory is not typically concerned with the computational complexity
  of queries, but our implementation must be.
  Indexing nodes in a large graph consumes memory, so SMTK allows
  arcs to be stored without data structures to traverse arcs in their
  "reverse" direction (i.e., from ``ToType`` nodes to ``FromType`` nodes).
  This indexing is **independent** of whether the arcs are considered
  directed or not.

+ When a graph is undirected and the nodes at each endpoint are of
  the same type (i.e., ``std::is_same<FromType, ToType>`` is true),
  then visiting the incoming arcs is identical to visiting the
  outgoing arcs. If you visit both the outgoing and incoming arcs
  incident to a node, you will iterate over all the (undirected)
  arcs twice.

  In this case, it is also important to note that constraints on the
  in-degree and out-degree must be identical – ``MaxOutDegree`` must be
  equal to ``MaxInDegree`` – since the same arc between nodes A and B
  may be stored as (A, B) or (B, A).

+ When a graph is undirected but requires nodes of different types at
  each endpoint, traversing the graph will require you to call both
  the incoming and outgoing arc visitors. This is because the C++
  callbacks for visiting nodes connected by arcs of these types must
  be passed a pointer to the proper node type.
