Attribute items have a "Reset to Default" context menu item
-----------------------------------------------------------

All the widget used to input values for attribute items have
a context menu item added called "Reset to Default", which will
change the value back to the default, if one is specified. If
no default is specified, the menu entry is disabled.

Int, Double, String, File, and DateTime items are all supported,
including spinbox and multi-line views. Resource items can't have
a default specified.

As a result, `qtInputItem` no longer set themselves back to the
default value when constructed - the default value is only
applied by the `ValueItem` when created, or when the user
chooses "Reset to Default".
