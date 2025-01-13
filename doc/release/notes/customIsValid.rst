Changes in Attribute Resource
=============================

Added ability to assign custom validity functions to Items
-----------------------------------------------------------

SMTK now supports the ability to assign a function to an Attribute Item that would
be used to determine if an Item is currently valid. ``Item::isValid`` method has been refactored
and most of the original logic has been moved to a new method called ``Item::defaultIsValid``.
To set a custom relevancy function, use ``Item::setCustomIsValid``.

To determine if an Item is valid you will still call ``isValid`` and it will use the custom validity
function if one has been set, otherwise it will call the default method.

Please see ``smtk/attribute/testing/cxx/customIsValidTest.cxx`` for an example of how to use this functionality.
