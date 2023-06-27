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

There is one known issue: the popup completer does not display if the numerical value ends
in the decimal point optionally followed by 1 or more zeroes.
