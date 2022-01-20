GraphTraits allows customized NodeStorage
--------------------------

Allow implementor to override the default container used for storing
node data.

NodeStorage API
~~~~~~~~~~~~~~~~~~

Developers may now implement custom node storage as an option in GraphTraits.

Implementations of ``NodeStorage`` must implement a minimal API to access and modify
the underlying node storage. Additional public APIs will be inherited by the
``smtk::graph::Resource``.

.. code-block:: c++

  class MinimalNodeStorage
  {
  public:
    /// Call a visitor function on each node in the graph
    void visit(std::function<void(smtk::resource::ComponentPtr>>& visitor) const;

    /// Find a node with a given uuid, if there is no node return a NULL pointer
    smtk::resource::ComponentPtr find(const smtk::common::UUID& uuid) const;

  protected:
    /// Erase a node from the node storage without updating the arcs
    bool eraseNode(const smtk::resource::ComponentPtr&);

    /// Insert a node to the node storage but do not check against NodeTypes
    bool insertNode(const smtk::resource::ComponentPtr&);
  };

Developer changes
~~~~~~~~~~~~~~~~~~~

Using the ``smtk::graph::ResourceBase::nodes()`` API is no longer available unless
the ``NodeStorage`` implements it.

The default ``NodeStorage``, ``smtk::graph::NodeSet``, provides an API for ``nodes`` and
is implemented using ``std::set<smtk::resource::ComponentPtr, CompareComponentID>`` as
the underlying container. This is consistent with the previous implementation and will
be automatically selected if no ``NodeStorage`` is specified in GraphTraits.
