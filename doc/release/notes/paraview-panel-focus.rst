Automated panel-switching fixed
-------------------------------

The move to separate panels from dockwidgets (cmb/smtk!2750) left code
that intended to show the current dock-widget dysfunctional (you must
call `→raise()` on the `QDockWidget`, not on the `QWidget` panel it
contains). This fixes situations, where the operation toolbox and the
parameter editor dock-widgets are tabbed together; selecting an
operation in the toolbox would not bring up the parameter editor panel.
Similarly, pressing Ctrl+Space (the "operation finder" shortcut) would
not bring up the operation toolbox.
