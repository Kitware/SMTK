## Attribute Resource Changes

### attribute::ItemDefinition::passCategoryCheck
attribute::ItemDefinition now has methods to compare its categories with a user provided set (or with respects to a single category).  If the input set of categories is empty then the method will always return true.  If the input set is not empty but the item's set of categories is then the method returns false.  Else the result will depend on the Definition's categoryCheckMode.

### attribute::ItemDefinition::categoryCheckMode
This can be set calling setCategoryCheckMode and influences the behavior of the passCategoryCheck method.  Its possible values are:

 * CategoryCheckMode::Any (Default) - at least one of its categories is in the input then passCategoryCheck returns true
 * CategoryCheckMode::All  - if all of its categories is in the input then passCategoryCheck returns true

### Other Changes
* FileSystemItem::ValueAsString() now returns "" when the item is not set.
