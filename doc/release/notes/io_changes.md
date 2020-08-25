## Changes to I/O Functionality
### Added Preliminary Support for Writing Attribute Libraries
The AttributeWriter can now be used to save a subset of Attributes based on a collection of Attribute Definitions.  This is the first step in supporting the creation of attribute libraries.  The new functionality include:

* includeAdvanceLevels - method to indicate if the AdvanceLevels section should be included in the saving process
* includeAnalyses - method to indicate if the Analysis section should be included in the saving process
* setIncludedDefinitions - Restricts the types of attribute instances written out to those derived from a specified list.  If the list is empty, then all attributes will be saved. Any redundant definitions (definitions that can be derived from others in the list) are removed.
* includedDefinitions - Returns the list of definitions to be used to filter attribute instances.  If empty then there is no attribute filtering
* treatAsLibrary - A convenience  method for creating a library. Write/WriteAsContents will produce a library like XML file containing only attribute instances that are based on the provided list of definitions. If the list is empty, then all attributes will be saved. This method will, by default, not include Analyses, AdvanceLevels, Definitions or View sections - these sections can be included by calling enabling them after calling this method.

### Added Support for Attribute Styles
This is supported in Version 4 XML Files.
**Note** Current Limitation: If writing out XML files to replicate the original include file structure, all of the style information will be saved in the top most file.  The reason for this is Style information read from included files can be over written in the more top-level files and there is currently no way to track this.

### Added attributeUtils
This is a place to store I/O utilities for attribute related information.

#### importFromCSV
 bool importFromCSV(smtk::attribute::GroupItem& item, const std::string& filename,
    Logger& logger, bool appendToGroup = false, const std::string& sep = ",", const std::string& comment = "")

 This function imports information from a CSV formatted file into a GroupItem that has the following characteristics:
 * It is extensible
 * All of its children are ValueItems
 * All of its children are not optional or extensible

 The function can ether overwrite the item's current groups or append to them.  Any input line that does not have the proper number of values, based on the item's structure, will be skipped and recorded in the logger.  Errors and warnings are returned through the logger.

#### importFromDoubleFile
 bool importFromDoubleFile(smtk::attribute::GroupItem& item, const std::string& filename,
  Logger& logger, bool appendToGroup = false, const std::string& optionalSep = ",", const std::string& comment = "")

 This function imports information from a file containing only doubles into a GroupItem that has the following characteristics:
 * It is extensible
 * All of its children are DoubleItems
 * All of its children are not optional or extensible

 The function can ether overwrite the item's current groups or append to them.  Any input line that has less than the proper number of values, based on the item's structure, will be skipped and recorded in the logger.  Errors and warnings are returned through the logger.
