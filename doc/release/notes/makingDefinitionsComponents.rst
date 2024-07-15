Changes to Attribute Resource
=============================
Attribute Definitions are now Resource Components
-------------------------------------------------

:smtk:`smtk::attribute::Definition` now derives from :smtk:`smtk::resource::Component`.
This means that Definitions now have UUIDs and may also have properties.

**Note** You do not need to specify UUIDs in either the XML or JSON file formats.  If an
ID is not specified, one will be assigned to it.

**Note** All previous JSON and XML files are supported by this change.  Version 8 XML and 7 JSON files and later
will support storing the IDs.  These formats will also support properties defined on Definitions.
As in the case for properties on the Resource and Attributes, the XML format only supports reading properties.

Developer changes
~~~~~~~~~~~~~~~~~~

** API Breakage: ** Since classes derived from the resource component class must provide a
method to return a shared pointer to a :smtk:`smtk::resource::Resource` instance via a member functions called
resource() and since the Definition class already had a method called resource() that returned a shared pointer
to its owning :smtk:`smtk::attribute::Resource`, this resulted in breaking API.  A new method called attributeResource()
was added to Definition that provides that same functionality as its original resource() method.  A simple name replacement is
all that is needed to resolve compilation errors resulting from this change.

:smtk:`smtk::attribute::Attribute::setId()` method was not being properly supported and now generates an error message if called.

The code used to parse property information in XML files has been relocated from the XMLV5Parser to its own file so it
can be reused.
