Added support for units in DoubleItem editor
---------------------------------------------

The default line editor for double items specified with units was changed to
include units in the text input, e.g., "3.14159 ft" or "2.71828 m/sec".
The new editor includes a dropdown completer listing the compatible units
for the item.
The list of compatible units are obtained from the unit system stored in the
attribute resource.
Double items specified without units use the same editor field as before.
The new editor is only used for numerical input, i.e., items with discrete or
expression options also use the same editor as before.

.. image ./UnitsUI.png

An example template file can be found at data/attribute/attribute_collection/unitsExample.sbt.

Known issues:

* The popup completer does not display if the numerical value ends in the decimal
  point optionally followed by 1 or more zeroes.
* The current logic treats unrecognized units as valid. This will be changed -- in
  the near future, SMTK will ignore units (strings) not recognized by the units system.
  A warning message will be output for each unrecognized unit.
* The "Restore Default" logic should use the definition's default string.
  (The current behavior is valid but unexpected.)
* Certain input strings are not detected and handled as invalid, e.g. "2 ftxx".
