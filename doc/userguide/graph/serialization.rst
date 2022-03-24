.. _smtk-graph-serialization:

Serialization
=============

The graph resource provides facilities to serialize itself to/from JSON.
Note that different subclasses of the graph resource have different serialization needs.

Some graph-resource subclasses (such as the OpenCascade session) use a third-party
modeling kernel (i.e., OpenCascade) to construct nodes upon load;
thus serialization should only include properties, links, and an array of preserved
UUIDs (in the order OpenCascade will iterate over them as it loads an OpenCascade file).
Similarly, arcs are not stored explicitly in the system but are programmatically
fetched by calling third-party APIs. Thus, no data should be serialized for arcs.

On the other hand, some graph-resource subclasses *should* store node and/or arc data.
An example of this is a markup resource, where
users create nodes and arcs explicitly and there is no third-party modeling kernel
that stores this data.
Therefore, markup resources need to serialize both arcs and nodes.

What SMTK provides
------------------

The graph-resource subsystem provides the ``to_json()`` and ``from_json()`` free functions
that the nlohman::json package expects for the base resource class;
however, you do not need to include or use them in implementing your own storage.
You must implement these free functions for your resource's node types.
Beyond this, the following rules apply:

+ Nodes are only serialized if they explicitly marked with a type-alias named ``Serialize``
  whose value evaluates to true (e.g., is aliased to ``std::true_type``).
+ Implicit arc types are not serialized (this may change in the future to allow
  developers to specify which implicit arcs should be serialized).
+ Explicit arc types are only serialized if they are mutable (because there would be
  no way to deserialize the arcs if not). See the ``Immutable`` arc trait documented
  above for more information.

The order of operations
-----------------------

The following outline illustrates the order in which deserialization occurs.
This order is important to understand as nodes must exist before explicit arcs
that connect them may be added.
Your read operation (let's call it ``your::session::Read``) should do the
following inside its ``operateInternal()`` implementation:

  + Your read operation creates a resource like so:

    ``resource = your::session::Resource<Traits>::create();``
  + You must inform the :smtk:`smtk::resource::json::Helper` that a new
    resource is being deserialized in the current thread, like so

    ``smtk::resource::json::Helper::pushInstance(resource);``
  + Then, you can import JSON data into the resource you created:

    ``your::sesssion::Resource<Traits>::Ptr resource = jsonData;``

    + The JSON library will eventually invoke your custom converter:

      ``some::from_json(const json& j, some::Resource<Traits>& resource);``

      + Inside your converter, you should assign the resource
        by fetching the value from the helper.
      + Load any third-party modeling-kernel data at this point.
      + Create any nodes that you do not expect to have been
        explicitly serialized by SMTK.
      + Explicitly invoke SMTK's graph-resource JSON conversion function like so:

        ``smtk::graph::from_json(j, resource);``

        + SMTK will explicitly invoke the base resource JSON coverter like so:

          ``smtk::resource::from_json(j, resource);``

          + This method will assign the resource's UUID, location,
            link, and property data.

        + Next, graph-resource nodes that have been explicitly serialized will be
          deserialized.
        + Finally, all arcs that have been explicitly serialized will be
          deserialized. At this point, all nodes that the arcs connect must exist.

  + You must inform the :smtk:`smtk::resource::json::Helper` that your
    resource deserialization is complete, like so:

    ``smtk::resource::json::Helper::popInstance();``

Example
-------

SMTK's test (TestArcs) provides a sample implementation to test serialization.
First, note that the base node type (``Thingy``) declares that it should be serialized:

.. literalinclude:: ../../../smtk/graph/testing/cxx/TestArcs.h
   :language: c++
   :start-after: // ++ 6 ++
   :end-before: // -- 6 --
   :linenos:

Because the only other node type (``Comment``) inherits the node type above and
does not override the ``Serialize`` type-alias, it will also be serialized.
Since node types may contain application-specific data, you are responsible
for providing free functions that serialize and deserialize your node types.

In the example, both node types inherit the same base class so this single
function is all that's required for serialization to JSON:

.. literalinclude:: ../../../smtk/graph/testing/cxx/TestArcs.cxx
   :language: c++
   :start-after: // ++ 4 ++
   :end-before: // -- 4 --
   :linenos:

Note that things are different for deserialization;
the free function is responsible for explicitly creating a shared pointer to
a node of the proper type.
Thus you must either provide an implementation for each class or a template
that fixes the node type at compile time.
The example uses the latter approach:

.. literalinclude:: ../../../smtk/graph/testing/cxx/TestArcs.cxx
   :language: c++
   :start-after: // ++ 5 ++
   :end-before: // -- 5 --
   :linenos:

The resource has one explicit arc type (which will be serialized since it is mutable)
and two implicit arc types (which will not be serialized).
SMTK automatically serializes arcs for you; no additional code is required for this.
