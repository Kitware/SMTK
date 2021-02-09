## Active Categories

The concept of active categories (categories that represent the active concepts of the work flow) have been added to the SMTK attribute system.
They are a set of category strings that are assigned to the Attribute Resource.  The new methods include:

* setActiveCategoriesEnabled - enables/disables active categories (default is false)
* activeCategoriesEnabled - returns the current active categories enable mode
* setActiveCategories - sets the set of active categories
* activeCategories - returns the current set of active categories
* passActiveCategoryCheck - methods to test Categories and Categories::Sets based on the Resource's active categories settings

### Other Changes
#### isValid Methods
Both Attribute and Item isValid() methods that use to take in no arguments now take in a bool to indicate if their Resource's active categories state should be used to determine validity.  The parameter is by default set to true.  Note that this does not change previous functionality since the Resource's default is disabled (aka false).

### isRelevant Methods
Both Attribute and Item now have an isRelevant method that returns true if they are relevant based on the Resource's active category state.  If the Resource has active categories disabled then these methods will always return true, else the method will test their categories based on the resource's active set.

### ReferenceItemDefinition
Added a new aspect called **enforcesCategories**.  In the case where a Reference Item is pointing to Attributes, this property means that the Item is valid iff the Attribute its pointing to is considered relevant (meaning it passes the current set of active categories).

### I/O Changes for both JSON and XML
* Added support for Active Categories
* Added support for enforcesCategories - **Note** that this represented by **EnforceCategories**  attribute in XML and as a Key in JSON
* See data/attribute/attribute_collection/refitem-categories.sbt for an example.

```xml
      <ItemDefinitions>
        <Component Name="material" EnforceCategories="true">
          <Accepts>
            <Resource Name="smtk::attribute::Resource" Filter="attribute[type='material']" />
          </Accepts>
        </Component>
      </ItemDefinitions>
```

### UI Changes
* All Category related methods have been removed from the UI Manager and now use the active category  mechanism
* qtBaseAttribute Category tests have been changed to take in a shared pointer to an Item instead of a reference to an ItemDefinition. - This was needed since ItemDefinitions do not have a way of getting access to the Resource.

#### qtReferenceItemComboBox Changes
* qtReferenceItemCombobox no longer uses "UseCategories" option in its ItemView.
* Now enforces the new **enforcesCategories** property
* When the item's value becomes invalid due to active category changes, this will now properly update the UI.
