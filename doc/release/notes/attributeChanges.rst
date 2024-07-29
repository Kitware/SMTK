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

Units Support for Definitions and Attributes
--------------------------------------------

You can now assign units to Definitions and Attributes.  This can be useful when an Attribute
represents a concept that has units but does not have an explicit value.  For example, if an
Attribute represents a temperature field that would be created by a simulation, you may want to
assign a default unit to its Definition (such as Kelvin) but allow the user to change the
Attribute's units to Celsius.

The rules for assigning local units to an Attribute that override those inherited through its definition
are the same as the case of assigning a value with units to a ValueItem.

Derived Definitions inherit units associated with their base Definition.  When overriding the units being inherited, by default a Definition's units must be compatible with the units coming from its base Definition, though the method provides an option to force units to be set even if they are not compatible.

Definitions whose units are "*" indicate that Definitions derived and Attributes that are create from can be assigned any supported units.
