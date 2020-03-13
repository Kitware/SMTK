## Changes to I/O Functionality
### Added Preliminary Support for Writing Attribute Libraries
The AttributeWriter can now be used to save a subset of Attributes based on a collection of Attribute Definitions.  This is the first step in supporting the creation of attribute libraries.  The new functionality include:

* includeAdvanceLevels - method to indicate if the AdvanceLevels section should be included in the saving process
* includeAnalyses - method to indicate if the Analysis section should be included in the saving process
* setIncludedDefinitions - Restricts the types of attribute instances written out to those derived from a specified list.  If the list is empty, then all attributes will be saved. Any redundant definitions (definitions that can be derived from others in the list) are removed.
* includedDefinitions - Returns the list of definitions to be used to filter attribute instances.  If empty then there is no attribute filtering
* treatAsLibrary - A convenience  method for creating a library. Write/WriteAsContents will produce a library like XML file containing only attribute instances that are based on the provided list of definitions. If the list is empty, then all attributes will be saved. This method will, by default, not include Analyses, AdvanceLevels, Definitions or View sections - these sections can be included by calling enabling them after calling this method.
