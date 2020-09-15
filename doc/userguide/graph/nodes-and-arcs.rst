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

As the first-class element of the graph structure, :smtk:`Component
<smtk::graph::Component>` (and types that derive from it) have access
to the parent resource and node id. The remainder of its API
(`contains()`, `set()`, `get()` and `visit()`) is described in the
context of the arc types that branch from the node. This separation of
API implementation from the node to the arc facilitates the traversal
of arcs or sets of arcs with different storage layouts without
modifying the node class description. In practice, the description of
collections of arcs in aggregate is often useful when describing a
relationship from one to many nodes that has common features (e.g., a
vertex can belong to several edges, or the edges of a face can be
described in counterclockwise order).

Classes describing arcs between nodes are expected to have a public
`alias` describing the node type at the tail of the arc
(`FromType`), the node type at the head of the arc (`ToType`), and a
class template describing the arc's API when it is accessed from a
node (the API is templated on the arc's `SelfType` to facililtate
derivation from arcs without having to redefine the API class for each
child arc type). The API class template provides methods `contains()`
and `get()` to check for the existence of an arc and to access the
elements at the head of the arc, respectively. Additionally, the API
class template can contain a `visit()` method that accepts a functor
that takes as input a node of type `ToType` and returns a bool; this
method should apply the functor to each node at the head of the arc,
returning from the iteration early if the functor returns true. If a
custom `visit()` method is not provided by an arc's API class
template, a default implementation for visiting the nodes at the head
of an arc is provided.
