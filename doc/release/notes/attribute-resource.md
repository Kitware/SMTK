# Changes to Attribute Resource
## DirectoryInfo and FileInfo
These classes were created so we can save out an attribute resource using the original include file structure.

A FileInfo instance represents the information found in an included attribute file which includes the following:

   + Catagories specified in the file
   + The set of include files that file uses
   + The default catagory that the file specified (if any)

The DirectoryInfo is simply a vector of FileInfos.  The IncludeIndex property found in Attributes, Definitions, and View corresponds to an index into this vector indicating that the instanced was read in from that file.

The first index in the array is "special" in that it represents the resource itself.  Any new Definitions, Attributes, or Views created after the resource is loaded into memory will be considered part of that "file".  This is why the default value for the IncludeIndex Property is 0!

## General Code Changes
+ attribute::System has been renamed to attribute::Resource
+ Added templated method smtk::attribute::Item::definitionAs<>() that will now returned the item's definition properly cast to the desired const item definition shared pointer type
+ Added associatedObjects() and disassociate() methods to smtk::Attribute to support non-model resources and resource components
+ Since attribute::ComponentItemDefinition changed it default behavior from SMTK 2.0 (it now creates a non-extensible item with a size of 1), a Definition needed to explicitly set the number of required values to 0 for its default association rule
+ Fixed bug in attribute::ReferenceItem that prevented values to be set to nullptr
+ Added an IncludeIndex property to Attribute and Definition.  This is used by I/O classes to represent include file structure
+ Added a DirectoryInfo Property to Attribute Resource.  This is used by I/O classes to represent the include file structure of the resource.
+ Added Resource::attributes(object) method to return all of the attributes associated with the persistent object that are owned by the attribute resource.
+ Added Definition::attributes(object) method to return all of the attributes associated with the persistent object that are derived from the definition or definitions derived from it.
+ Added ReferenceItem::acceptableEntries() method to return it's definition's acceptability rules.
+ Added Protected method ReferenceItemDefinition::setRole(role) method for setting the role of its related resource links.
