Key Concepts
============

Internally, SMTK represents a workflow as a directed acyclic graph.
Nodes in the graph may be

+ actions to take;
+ conditions which must be met; or
+ choices between acceptable actions or conditions.

Edges in the graph indicate dependencies between nodes.

Presenting a complete task DAG would be confusing to many users.
Instead, we use SMTK's view presentation model to allow applications
to tailor what is shown based on the current state.
