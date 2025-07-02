Mesh Subsystem
==============

The mesh subsystem (which introduced a dependency on the MOAB library
which is no longer maintained) has been removed from SMTK.
In practice, we have also realized mesh data using other resource
types (such as the VTK or markup sessions); either we will continue
with this pattern in the future (implementation-dependent mesh resources
as needed) or develop a mesh abstraction that is not tied so closely
to a particular implementation.
