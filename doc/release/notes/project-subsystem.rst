Changes to Project Subsystem
============================

A major revision was undertaken to the ``smtk::project`` namespace that is not
backward compatible. The previous Project class was hard-coded to a use single
attribute resource and one or two model resources. The new Project class is an
SMTK resource that can support any number of resources including custom ones,
with assigned roles. The new Project class can optionally be scoped to a list
of available operations and resource types. New Project instances store and
retrieve their contents as atomic operations with the file system.
Projects can now be described entirely using Python, making it
easier to implement custom project types for different solvers.

More information is available in the SMTK user guide and there is also a new
``create_a_project`` tutorial.
