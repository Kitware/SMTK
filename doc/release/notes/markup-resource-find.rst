Finding markup components by name
---------------------------------

Finding components in a markup resource by name is now supported.

This also implicitly supports searches by both type
and name since the output container type is templated
and objects will be cast to the container's value-type
before insertion.

Currently, output containers must hold raw pointers to nodes
rather than weak or shared pointers to nodes. This may
change in future versions.
