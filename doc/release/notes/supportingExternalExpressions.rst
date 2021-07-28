Supporting "External" Expressions
=================================
There are use cases where the workflow may want to store expressions in a separate Attribute Resource.
The core of SMTK already supported this but the UI system assumed that the Attribute Resource which owned the ValueItem was also the source for expressions.  This is no longer the case.

qtInstancedView can now optionally take in an Attribute Resource instead of solely relying on the one associated with the UI Manager.  This allows classes like the qtAttributeEditor to supply the Attribute Resource.

Added a new query function called: findResourceContainingDefinition that will return an Attribute Resource that contains an Attribute Definition referred to by its typename.  If the optional Attribute Resource provided to the function also contains the Definition, it is returned immediately without doing any additional searching.  This maintains the original use case where the expressions are stored in the same resource.

qtInputItem no longer assumes the Item's Attribute Resource is the one being used as a source for expressions.

Added two template files that can be used to demo the functionality.

data/attribute/attribute_collection/externalExpressionsPt1.sbt - Contains an Attribute Definition with an Item that can use an expression that is not defined in that template

data/attribute/attribute_collection/externalExpressionsPt2.sbt - Contains an Attribute Definition that represents the expressions used in Pt1.

Closes #439
