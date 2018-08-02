# Changes to Attribute Resource
+ attribute::System has been renamed to attribute::Resource
+ Added templated method smtk::attribute::Item::definitionAs<>() that will now returned the item's definition properly cast to the desired const item definition shared pointer type
+ Added associatedObjects() and disassociate() methods to smtk::Attribute to support non-model resources and resource components
+ Since attribute::ComponentItemDefinition changed it default behavior from SMTK 2.0 (it now creates a non-extensible item with a size of 1), a Definition needed to explicitly set the number of required values to 0 for its default association rule
+ Fixed bug in attribute::ReferenceItem that prevented values to be set to nullptr
