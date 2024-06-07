Changes in Attribute Resource
=============================

Added ability to assign custom relevancy functions to Items
-----------------------------------------------------------

SMTK now supports the ability to assign a function to an Attribute Item that would
be used to determine if an Item is currently relevant.  ``Item::isRelevant``
method has been refactored and most of the original logic has been moved to a new method called ``Item::defaultIsRelevant``.
To set a custom relevancy function, use ``Item::setCustomIsRelevant``.

To determine if an Item is relevant you will still call ``isRelevant`` and it will use the custom relevancy function if one has been set, else it will call the default method.

Please see ``smtk/attribute/testing/cxx/customIsRelevantTest.cxx`` for an example of how to use this functionality.
