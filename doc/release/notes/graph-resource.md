## Graph-Model Resource

This release of SMTK includes a flexible new approach to modeling;
rather than a fixed set of model components which assume a CAD-style
boundary representation, the `smtk::graph::Resource` allows an
extensible set of components (nodes) connected by relationships (arcs)
that are constrained by the types of nodes at their endpoints.

See the user's guide, the unit tests, and the OpenCASCADE session for
more information.
