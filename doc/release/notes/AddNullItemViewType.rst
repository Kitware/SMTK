Added the ability to hide an Attribute's Item In a View
-------------------------------------------------------

SMTK's AttributeItemViews can now support *null* type.  By setting **Type="null"**
in an Attribute Item View, that specific item (and of its related children) will not be
displayed in a View.

Please look at data/attribute/attribute_collection/NullItemViewExample.sbt for an
example that uses this capability.

Developer changes
~~~~~~~~~~~~~~~~~~

In order to cleanly support this, qtAttributeItemInfo now provides a *toBeDisplayed* method that will
return true iff all of the following conditions are met:

* The instance has a valid smtk::attribute::item
* There is either no baseView or the baseView indicates that the item should be displayed
* There is either no ItemView Configuration or that its type is not set to *null*
