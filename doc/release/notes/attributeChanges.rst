Changes to Attribute Resource
=============================

Added `attribute::Resource::hasDefinition(type)` Method
------------------------------------------------------

This method will return true if the resource already contains a Definition with
the requested type.

Added `attribute::Analyses assignment method
--------------------------------------------

You can now assign the contents of one Analyses instance to another.  This will create copies
of all of the Analysis instances owned by the source Analyses.
