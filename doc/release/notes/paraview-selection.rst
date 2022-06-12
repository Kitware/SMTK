Selection filtering for graph resources
---------------------------------------

A bug in the pqSMTKSelectionFilterBehavior class has been fixed.
It prevented the default selection-responder operation from adding
graph nodes to the selection (basically, anything that could not be
cast to a model entity was rejected).

Now the selection filter toolbar buttons only apply to model and
mesh entities; graph-resource components will always be passed
through.
