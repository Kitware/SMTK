## Remove ModelEntityItem and MeshItem

+ ModelEntityItem and MeshItem are now removed from SMTK.
+ Attribute associations are now managed as smtk::attribute::ReferenceItem instances.
  This is a significant change.
  Previously, there was no way to association non-model-entity objects to attributes.
  Now, any smtk::resource::PersistentObject (including both component *and* the resources
  that own components) may be associated with an attribute, assuming that the attribute
  permits it.
  This flexibility does mean that some dynamic casting may be required that was not before.
+ Operators that used ModelEntityItem or MeshItem now use either
  ReferenceItem or ComponentItem instead.
+ The standard operator result items (_created_, _expunged_, _modified_) are now
  ComponentItem instances.
+ You may now call `setValuesVia(begin, end, converter, offset)` and
  `appendValuesVia(begin, end, converter)` on ReferenceItem (and its subclasses).
  The converter is a C++ lambda called on each value iterated and should
  return `smtk::resource::PersistentObjectPtr`.
