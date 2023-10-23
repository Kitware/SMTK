Graph system
------------

Query grammar
~~~~~~~~~~~~~

The graph-resource :smtk:`query grammar <smtk::graph::filter::Grammar>` has
been extended to allow "bare" component type-names.

For example, if your filter-query was ``'SomeNodeType' [string{'name'='foo'}]``,
it is now also legal to write ``SomeNodeType [string{'name'='foo'}]`` (i.e., no
single-quotes required around the node's type-name).
This simplifies some upcoming changes for run-time arcs.
