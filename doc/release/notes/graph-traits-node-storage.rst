GraphTraits allows customized NodeContainer
--------------------------

Allow implementor to override the default container used for storing
node data.

NodeContainer API
~~~~~~~~~~~~~~~~~~

Developers may now implement custom node storage as an option in GraphTraits.

Implementations of ``NodeContainer`` must implement a minimal API to access and modify
the underlying node storage. Additional public APIs will be inherited by the
``smtk::graph::Resource``.

.. code-block:: c++

  class MinimalNodeContainer
  {
  public:
    // Implement APIs inherited from smtk::resource::Resource.

    /** Call a visitor function on each node in the graph.
     */
    void visit(std::function<void(smtk::resource::ComponentPtr>>& visitor) const;

    /** Find the node with a given uuid, if it is not found return a nullptr.
     */
    smtk::resource::ComponentPtr find(const smtk::common::UUID& uuid) const;

  protected:
    // Implement protected APIs required by smtk::graph::Resouce and smtk::graph::Component.

    /** Erase all of the nodes from the \a node storage without updating the arcs.
     *
     * This is an internal method used for temporary removal, modification, and
     * re-insertion in cases where \a node data that is indexed must be changed.
     * In that case, arcs must not be modified.
     *
     * Returns the number of nodes removed. Usually this is either 0 or 1, however the
     * implementation may define removal of > 1 node but this may cause unintended behavior.
     */
    std::size_t eraseNodes(const smtk::graph::ComponentPtr& node);

    /** Unconditionally insert the given \a node into the container.
     *
     * Do not check against NodeTypes to see whether the node type is
     * allowed; this has already been done.
     *
     * Returns whether or not the insertion was successful.
     */
    bool insertNode(const smtk::graph::ComponentPtr& node);
  };

Developer changes
~~~~~~~~~~~~~~~~~~~

The Graph API no longer accepts storing nodes that are not derived from
``smtk::graph::Component``. This is enforced by the APIs required from the
NodeContainer.

Using the ``smtk::graph::ResourceBase::nodes()`` API is no longer available unless
the ``NodeContainer`` implements it.

The default ``NodeContainer``, ``smtk::graph::NodeSet``, provides an API for ``nodes`` and
is implemented using ``std::set<smtk::resource::ComponentPtr, CompareComponentID>`` as
the underlying container. This is consistent with the previous implementation and will
be automatically selected if no ``NodeContainer`` is specified in GraphTraits.

Note, a ``smtk::resource::ComponentPtr`` is used in the underlying storage to prevent having
to cast pointers for APIs inherited from ``smtk::resource::Resource``.
