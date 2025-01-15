ParaView Extensions
===================

Operation Toolbar
-----------------

SMTK now provides :smtk:`pqSMTKOperationToolbar`, which you can
inherit to add a toolbar to your application that includes buttons
for SMTK operations.

Operations that have parameters which are valid by default
will be launched immediately as the button is clicked;
operations whose parameters must be edited will result in
the operation parameter-editor panel being raised to edit
parameters.

Qt Extensions
=============

The :smtk:`qtOperationTypeModel` class has been extended to insert
buttons into a provided toolbar.
This is used by the :smtk:`pqSMTKOperationToolbar` class to
insert toolbar buttons using the active server's operation-model.
