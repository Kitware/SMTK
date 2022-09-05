Graph-resource Filter Grammar
-----------------------------

The query-filter string-parser for the graph-resource had
a bug where parsing would succeed with some incorrect grammars
because the parser was not forced to consume the entire string
to obtain a match; a partial match would succeed but not produce
a functor that evaluated graph nodes properly.
This has been fixed, so error messages should now be emitted
when a filter-string is ill-formed.
