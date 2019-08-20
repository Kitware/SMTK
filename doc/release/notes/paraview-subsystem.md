## Changes to the ParaView UI subsystem

### Widgets

The box widget (pqSMTKBoxItemWidget) now supports a binding that allows
the visibility of the widget to be mapped to a discrete-valued string
item with enumerants "active" and "inactive".

### Subtractive UI

You may now subtract basic ParaView UI elements (QActions
such as toolbar buttons and menu items) by calling methods
on the pqSMTKSubtractiveUI class during your plugin's
initialization (or at later times as needed).

Note that

+ You should not attempt to remove UI elements added by
  other plugins since the order in which plugins are
  loaded is unspecified.
+ You may disable and re-enable items in response to
  changes in the interface (e.g., the CMB "post-processing"
  plugin, which disables the "Sources" and "Filters" menu
  items at startup but re-enables them when users enter
  post-processing mode.

## Selection

Operations are now used to translate VTK/ParaView selections
into SMTK selections. See the user guide for details.
This change was made to support mesh and instance subset selections.
