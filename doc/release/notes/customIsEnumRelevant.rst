Changes in Attribute Resource
=============================

Added ability to assign custom enum relevancy functions to Value Item Definitions
---------------------------------------------------------------------------------

SMTK now supports the ability to assign a function to a Value Item Definition that would
be used to determine if a Value Item's discrete enumeration value  is currently relevant.

Please see ``smtk/attribute/testing/cxx/customIsRelevantTest.cxx`` for an example of how to use this functionality.
