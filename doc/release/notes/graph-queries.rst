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
Single-quoted component names imply an exact match to the object type,
while bare type-names also match derived objects.
For example, if class ``B`` inherits ``A``, then a filter-query ``'A'`` will only
match instances of ``A`` and not ``B`` while ``A`` will match instances
of ``B`` as well.

This testing of derived types is accomplished
by checking whether a query-filter token is present in
a `std::unordered_set<smtk::string::Token>` computed once at run-time, so
it is efficient.
