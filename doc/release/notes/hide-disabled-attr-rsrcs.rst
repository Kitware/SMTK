Hide disabled attribute resources
=================================

The Attribute Editor panel will now first check to see if an
Attribute Resource is enabled before attempting to display it.
Telling the Attribute Editor panel to display a disabled Attribute
Resource will be the equivalent to telling the panel to display a
nullptr.  The panel will be reset if it was currently display
any widgets.
