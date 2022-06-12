Graph-resource Dump improvements
--------------------------------

While :smtk:`smtk::graph::Resource`'s dump() method is unchanged,
if you use ``resource->evaluateArcs<Dump>()`` explicitly it is
now possible to set a color per arc type and to enable a
transparent background for the graph.
(This applies only to graphviz-formatted output.)
