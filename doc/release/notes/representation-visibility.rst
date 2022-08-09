ParaView representation API extended
------------------------------------

The client-side :smtk:`pqSMTKResourceRepresentation` now has a
method named ``allVisibilities()`` that returns the current
visibility (in the representation's view) of each component
by updating a map you pass.

This is intended to be used by upcoming classes that will
replace the visibility badge with versions that properly
reflect hierarchical visibility of sub-components.
