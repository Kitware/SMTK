Resource system
===============

Unit system API name change
---------------------------

The methods to set or get the system of units for a resource have
been renamed to match the terminology most engineers use: "unit"
is singular since there is one system holding all of the units.

+ :smtk:`unitsSystem()<smtk::resource::Resource::unitsSystem()` becomes
  :smtk:`unitSystem()<smtk::resource::Resource::unitSystem()>`
+ :smtk:`setUnitsSystem()<smtk::resource::Resource::setUnitsSystem()` becomes
  :smtk:`setUnitSystem()<smtk::resource::Resource::setUnitSystem()`

The attribute resource has additional methods that have changed:

+ :smtk:`Definition::setItemDefinitionUnitsSystem()<smtk::attribute::Definition::setItemDefinitionUnitsSystem()` becomes
  :smtk:`Definition::setItemDefinitionUnitSystem()<smtk::attribute::Definition::setItemDefinitionUnitSystem()`
+ :smtk:`ItemDefinition::setUnitsSystem()<smtk::attribute::Definition::setUnitsSystem()` becomes
  :smtk:`ItemDefinition::setUnitSystem()<smtk::attribute::Definition::setUnitSystem()`
