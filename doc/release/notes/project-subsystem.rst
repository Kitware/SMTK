# Changes to Project Subsystem

A major revision was undertaken to the `smtk::project` namespace that is not
backward compatible. The previous Project class was hard-coded to a use single
attribute resource and one or two model resources. The new Project class can
support any number of resources including custom ones, and can optionally be
configured to only support a whitelist set of resource types.
