Changes To Attribute Resource
=============================

Changes to ItemPath Related Methods
-----------------------------------

Attribute::itemPath is being deprecated and replaced by Item::path.  Previously Attribute::itemPath did not properly handle Items that were contained within the sub-groups of a GroupItem; Item::path does.  Note that the deprecated does call Item::path under the covers.  The main reason for the deprecation was that the functionality only depends on the Item and forcing developers to get the Item's Attribute in order to get its path was not necessary and potentially costly

Attribute::itemAtPath was changed to be completely compatible with Item::path.  The original issues was the interpretation of the separator string parameter.  In the methods that return the path, the separator was inserted between the components of the path while in Attribute::itemAtPath, it was interpreted as a list of characters where any character in the string would be considered a separator.  This change forces Attribute::itemAtPath to use the interpretation of Item::path.

Also added a unit test called ``unitItemPath.cxx``.

Also updated qtInstanceView to use Item::path when referring to modified Items instead of just their names.  This makes it consistent with qtAttrubuteView.
