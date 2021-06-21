Attribute itemPath Method
-------------------------

Added "smtk::attribute::Attribute::itemPath" method that will return the full path string to the "item"
parameter within the attribute. The default separator is "/" and can be
changed as needed.

qtGroupItem's Modified Children
-------------------------------
When the user modifies an item that is a child of a group item from the gui,
the full path to that item is included in the result of the resulting Signal
operation that will be run by the view.
