Added support for units in DoubleItem editor
---------------------------------------------

The default line editor for double items specified with units was changed to
include units in the text input, e.g., "3.14159 ft" or "2.71828 m/sec".
The new editor includes a dropdown completer listing the compatible units
for the item.
The list of compatible units are obtained from the unit system stored in the
attribute resource.
Double items specified without units use the same editor field as before.
Double items specified with units that are not recognized by the units system
also use the same editor as before.
Items with discrete options also use the same editor as before.

Where the new units-aware UI is used, the label no longer includes the units
string. It is replaced with a placeholder string when the field is empty
and the units completer otherwise.

.. image ./UnitsUI.png

An example template file can be found at data/attribute/attribute_collection/unitsExample.sbt.

The results display for infix expressions was also updated to append the units string
for the case where the units are recognized by the units system.
