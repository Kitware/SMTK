Runtime Arcs
============

Beyond arcs provided at compile-time as described above, it is also
possible to add new types of arcs at run time.
Since these arcs must be serializable (along with any logic used to
re-add them when loading in a resource that uses them),
a compile-time helper template named
:smtk:`RuntimeArc <smtk::graph::RuntimeArc>` is provided.
It is templated on :smtk:`Directionality <smtk::graph::Directionality>`,
which determines whether the run-time arc is directed or not.

If you want your resource to allow directed or undirected
run-time arcs, you must add one or both templates to your resource's
``Traits`` object (in the ``ArcTypes`` tuple).

The :smtk:`RuntimeArc <smtk::graph::RuntimeArc>` template
accepts a set of string tokens that constrain the types of nodes
that the arc is allowed to connect.

In the future, it may also accept a Python code snippet used to
validate connections, but this could be problematic if Python
support is not built into SMTK on machines that need to process
your resource.

Adding run-time arc types
~~~~~~~~~~~~~~~~~~~~~~~~~

To add a new arc type at run time, use the ``insertRuntimeArcType`` method
on your resource's :smtk:`ArcMap <smtk::graph::ArcMap>`:

.. code:: cpp

   smtk::string::Token arcTypeName = "Example"_token;
   std::unordered_set<smtk::string::Token> fromTypes;
   std::unordered_set<smtk::string::Token> toTypes;
   auto directionality = Directionality::IsDirected; // or Undirected

   resource->arcs().insertRuntimeArcType(
     arcTypeName, fromTypes, toTypes, directionality);

If you pass empty sets for ``fromTypes`` or ``toTypes`` (as in the example
above), then no type-checking is performed and the arc may connect nodes
of any type.
