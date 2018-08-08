# Changes to IO Processing
+ Refactored out the processing of association information when reading in attribute::Definitions so it can be overridden
+ Implemented missing support for V1/V2 association information for attribute::Definition
+ Supporting the ability to write out resources using included files.  This allows the possibility of creating a program to update existing attribute template files to the latest version.  Currently the attributeReaderWriterTest has been modified for this very purpose.
+ Removed the ResourceSet Reader and Writer classes - in SMTK 3.0 they should no longer be needed
