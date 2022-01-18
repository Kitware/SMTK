Graph Arcs/OrderedArcs use WeakReferenceWrapper
--------------------------

The type used for storing arc ``ToTypes`` has been changed from a
``std::reference_wrapper<ToType>`` to a new type
``smtk::common::WeakReferenceWrapper<ToType>`` in order to communicate to the
arc that the node component of the edge has been removed. This allows nodes to
be removed and edges to update lazily.

Developer changes
~~~~~~~~~~~~~~~~~~

Plugins using SMTK graph infrastructure will need to rebuild and fix type errors
to match the new implementationn in SMTK. They will also need to make sure they
are checking if ``to`` nodes in an arc are valid using the
``smtk::common::WeakReferenceWrapper<ToType>::expired()`` API to dectect if the
node is expired or not before accessing it.

Previously, if a node was removed, access via a ``to`` node in an arc would
silently fail or sefault. Now, invalid access will result in a
``std::bad_weak_ptr`` exception when attempting to access the expired data.
