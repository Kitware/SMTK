## Attribute Resource Changes

### attribute::ItemDefinition::passCategoryCheck
attribute::ItemDefinition now has methods to compare its categories with a user provided set (or with respects to a single category).  If the input set of categories is empty then the method will always return true.  If the input set is not empty but the item's set of categories is then the method returns false.  Else the result will depend on the Definition's categoryCheckMode.

### attribute::ItemDefinition::categoryCheckMode
This can be set calling setCategoryCheckMode and influences the behavior of the passCategoryCheck method.  Its possible values are:

 * CategoryCheckMode::Any (Default) - at least one of its categories is in the input then passCategoryCheck returns true
 * CategoryCheckMode::All  - if all of its categories is in the input then passCategoryCheck returns true

### Category dependent attribute::isValid method added
 This method will base the attribute's validity on a set of categories that are used to filter out items whose validity are to be ignored.

### Supporting Unique Roles for ComponentItems
There are use cases where the developer would like to enforce a constraint among ComponentItems such that each item cannot point to the same resource component. In order to provide this functionality, we have introduced the concept of unique roles.  Roles in this context refers to the roles defined in the resource links architecture and that are referenced in ReferenceItemDefinition.  You can now specify the role to be used for the ReferenceItemDefinition and add that role to the attribute::Resource's set of unique roles using attribute::Resource::addUniqueRole().

When assigning a component to a ComponentItem using a unique role, the item will test the value using its own isValueValid method that takes into consideration its current state and will check to make sure there are no other component items (using the same role) are associated with the component.

The following API have been added/changed to support this feature:

* New methods for smtk::attribute::Resource
	*  void addUniqueRoles(const std::set\<smtk::resource::Links::RoleType>& roles);
	* void addUniqueRole(const smtk::resource::Links::RoleType& role);
	* const std::set\<smtk::resource::Links::RoleType>& uniqueRoles() const;
	* bool attribute::Resource::isRoleUnique(const smtk::resource::Links::RoleType& role) const;
	*  smtk::attribute::AttributePtr findAttribute(const smtk::resource::ComponentPtr& comp, const smtk::resource::Links::RoleType& role) const;
* New methods for ComponentItem
	* virtual bool isValueValid(std::size_t ii, const ComponentPtr entity) const;
	* bool isValueValid(const ComponentPtr entity) const;
* ReferenceItemDefinition::setRole has been made public


### Other Changes
* FileSystemItem::ValueAsString() now returns "" when the item is not set.
* When Attribute::Attribute(...) no longer creates the attribute's items.  This is now done using the new Attribute::build() method - this allows Items to access the attribute's shared pointer when they are constructed.
* ReferenceItem now unsets it's values when being deleted so the corresponding links are removed from the resource.
	* In order for ReferenceItem to unset its values, it now holds onto a weak pointer to the attribute rather than using the attribute() method.  The reason is that Items that are owned by other Items lose their connection to the attribute when being deleted.  This ensures that the ReferenceItem will be able to access the attribute.

### Bug Fixes
* Attributes where not properly release it's association information when being deleted or when updating it's association information during Definition::buildAttribute
