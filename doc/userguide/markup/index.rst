.. _smtk-markup-sys:

-----------------------------------
SMTK's Markup (Annotation) Resource
-----------------------------------

Building on :ref:`smtk-graph-sys`, the markup resource provides
nodes and arcs typical of an annotation workflow. Where the graph
resource makes no assumptions about nodes and arcs, the markup
resource provides discrete geometric modeling entities (image data,
unstructured data, subsets, and sidesets), groups, and relationships
to a formal ontology.

The markup resource is intended as

1. a reference for how to subclass the graph subsystem;
2. a resource type for annotating geometric models;
3. a base class for domain-specific annotation.

In the final case, derived resources would be expected to add new node
and arc classes; to add operations, queries, and filter grammars; and
to subset the existing markup operations exposed to users (via an
:smtk:`OperationDecorator <smtk::view::OperationDecorator>`).

.. toctree::
   :maxdepth: 3

   concepts.rst
