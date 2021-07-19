Supporting smtk.extensions.attribute_view.name_read_only in qtAttributeViews
============================================================================
You can now indicate that an Attribute's name should not be modified by creating a bool Property on the Attribute called: smtk.extensions.attribute_view.name_read_only and setting its value to true.

Observing Operations
====================
qtAttributeView will now properly examine modified attributes to see if they have smtk.extensions.attribute_view.name_read_only property or if their names had been changed.
