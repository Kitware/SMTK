Attribute Resource Changes
==========================

Supporting Explicit Units for DoubleItems
-----------------------------------------

DoubleItems can now have units explicitly assigned to them provided that their Definition does not
specify units.  This allows Items coming from the same Definition to have different units.

Modified API
~~~~~~~~~~~~

* ``DoubleItemDefinition::hasSupportedUnits`` has been moved to ValueItemDefinition

Added API
~~~~~~~~~

* ValueItem
  * units() - returns the native units for the item
  * supportedUnits() - returns the supported units for the item.  If there is no Units System assigned to its definition or if its units are supported by the Units System, an empty string is returned else it returns its units.
* ValueItemDefinition
  * supportedUnits() - similar in concept as ValueItem's
* DoubleItem
  * setUnits() - explicitly sets the units of the item
  * units() - overridden to support explicit units
  * hasExplicitUnits() - returns true if the item has explicit units.

When changing the units of an Item, the system will see if the Item's current input string values are compatible, if they are not, the input value units are replaced with the new ones.

See smtk/attribute/testing/cxx/unitDoubleTest.cxx for an example.

Both XML and JSON formats have been updated to support this functionality as well as qtInputsItem.cxx, qtDoubleUnitsLineEdit{.h, .cxx}.
