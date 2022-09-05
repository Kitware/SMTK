Property editor operation
-------------------------

SMTK now provides a freeform property editor
named :smtk:`smtk::operation::EditProperties`
you can use to both inspect and edit integer,
string, floating-point, and coordinate-frame
properties on any component.

The custom operation view monitors the SMTK
selection provided to the Qt UI manager for
changes and updates the set of components being
inspected/edited.
