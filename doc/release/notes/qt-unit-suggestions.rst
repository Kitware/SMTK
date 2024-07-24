Qt extensions
=============

Double-item with unit completion
--------------------------------

The ``qtDoubleUnitsLineEdit.cxx`` class now uses the unit-library's
``PreferredUnits`` class to suggest unit completions. This allows
workflow developers to provide a tailored list of suggested completions
rather than listing every available compatible unit.

Line Edit widget with unit completion
-------------------------------------

The ``qtUnitsLineEdit.cxx`` class is for entering strings that represent units.
The widget provides a units aware completer as well as color coding its background
based on the entry being a valid unit or if the unit is the default.
as in the case of  ``qtDoubleUnitsLineEdit.cxx`` class, it can make use of the unit-library's
``PreferredUnits`` class to suggest unit completions. This allows
workflow developers to provide a tailored list of suggested completions
rather than listing every available compatible unit.

Displaying Attribute with Units
-------------------------------
Using the new ``qtUnitsLineEdit`` class, end-users can now set units on an Attribute (assuming that its Definition supports units).
This required changes to ``qtAttribute`` to not consider an Attribute *empty* if it had no items to display but did have specified units.

New XML Attribute for controlling how Units are displayed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use UnitsMode to indicate how an Attribute's units should be displayed.

* none - do not display the attribute's units
* viewOnly - display but do not allow the units to be changed
* editable - allow the user to edit the attribute's units

**Note** that these values are case insensitive

See DoubleItemExample.sbt as an example that demonstrates this new functionality.

Changes to ``qtBaseAttributeView`` and Derived View Classes
------------------------------------------------------------

* Added displayAttribute method that returns true if the attribute should be displayed based on its relevance.
* Changed displayItem to take in a ``const smtk::attribute::ItemDefinitionPtr&`` instead of a ``smtk::attribute::ItemDefinitionPtr`` **Note** that this does break API though it is very simple to update to the new API
