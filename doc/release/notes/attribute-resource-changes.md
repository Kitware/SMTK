## Changes to Attribute Resource
### Supporting Required Analyses
A smtk::attribute::Analyses::Analysis can now be marked as **required**.  A required Analysis indicates that it is not optional and is considered active if its parent analysis is active (or the analysis is at the top level).  The following methods have been added to smtk::attribute::Analyses::Analysis:

* setRequired(bool)
* bool isRequired() const

The following is a example setting an Analysis to be required via a sbt file:

```xml
    <Analysis Type="Required Analysis" Required="true"/>
```
See smtk/data/attribute/attribute_collection/SimpleAnalysisTest.sbt for a complete example.

**Note** that an Analysis's parent has Exclusive Children then the Analysis' required property is ignored.

### Replacing updateCategories method
The new method is now called finalizeDefinitions since it now does more including updating advance level information.

### Adding Advance Level and Category Support for DiscreteItem Enums
A Discrete Item's enums can now have a set of categories associated with it as well as advance level information.  This information is used by the GUI system to filter out enums based on category and advance level settings.

#### New ValueItemDefinition Methods
* setEnumCategories(const std::string& enumValue, const std::set\<std::string>& cats);
* addEnumCategory(const std::string& enumValue, const std::string& cat);
* std::set\<std::string> enumCategories(const std::string& enumValue) const;
* const std::map\<std::string, std::set\<std::string>> enumCategoryInfo();
* void setEnumAdvanceLevel(const std::string& enumValue, unsigned int level);
* void unsetEnumAdvanceLevel(const std::string& enumValue);
* unsigned int enumAdvanceLevel(const std::string& enumValue) const;
* bool hasEnumAdvanceLevel(const std::string& enumValue) const;
* const std::map<std::string, unsigned int> enumAdvanceLevelInfo() const;

#### IO Support
Both JSON and XML IO has been updated to support the new functionality.

In terms of XML the following shows an example snippet for using the new capabilities:

```xml
       <String Name="s1" Label="Advance Level and Enum Test String" Version="0" OkToInheritCategories="true" CategoryCheckMode="Any" NumberOfRequiredValues="1">
        <Categories>
          <Cat>s1</Cat>
        </Categories>
        <DiscreteInfo>
          <Structure>
            <Value Enum="e1" AdvanceLevel="1">a</Value>
            <Categories>
              <Cat>ec1</Cat>
            </Categories>
          </Structure>
          <Structure>
            <Value Enum="e2">b</Value>
            <Categories>
              <Cat>ec2</Cat>
            </Categories>
          </Structure>
          <Value Enum="e3" AdvanceLevel="1">c</Value>
        </DiscreteInfo>
      </String>

```
See smtk/attribute/testing/cxx/unitCategoryTest.cxx and smtk/data/attribute/attribute_collection/ConfigurationTest.sbt for examples.

### Support for ReferenceItems within detached Attributes
When an attribute containing ReferenceItems (including associations)
is detached, the links describing the connections between the
ReferenceItems and their references are now severed (originally, this
was only true for ReferenceItems representing associations). The ReferenceItems'
caches remain populated after detachment to support the
ReferenceItems' API once its parent attribute is removed from its
Resource.

### Other Changes
#### Attribute::isValid and Item::isValid Methods
* Passing an empty set of categories to Attribute::isValid or Item::isValid no longer means don't filter on categories.  Instead the methods will always return false.  If you don't want category filtering call the isValid methods that don't take in the set.
* Item::isValid method that taken in categories is no longer virtual
* A new virtual method Item::isValidInternal which performs the actual check has been added

### isSet() no longer asserts or exhibit undefined behavior
Previously calling isSet() for items that had its numberOfValues() == 0 would either result in an assert or indexing a vector of size 0.  This has been fixed so calling the method for an index that doesn't exist will always return false.
### unset() will now assert for indices that don't exist
Previously some Items would exhibit undefined behavior but will now simply assert.
### Added Item::localEnabledState() method
This method returns the the value of the Item's enabled flag. **Note** This is different from Item::isEnabled() method that also takes into consideration the enabled state of the Item's parent.
### Added Tags support for ItemDefinitions
This is identical to the functionality provided by Definitions.  Here is an example in XML.

```xml
 <Definitions>
    <AttDef Type="att1" Label="att1" BaseType="" Version="0" Unique="false">
      <Tags>
        <Tag Name="My Tag" />
        <Tag Name="My Tag with Values">value1,value2,value3</Tag>
      </Tags>
      <ItemDefinitions>
        <String Name="str" Label="str" Version="0" OkToInheritCategories="true" CategoryCheckMode="Any" NumberOfRequiredValues="1">
          <Tags>
            <Tag Name="Empty Tag" />
            <Tag Name="Str Tag with Values">v1,v2,v3</Tag>
          </Tags>
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
```

See smtk/attribute/testing/cxx/unitDefinitionTags.cxx and smtk/attribute/testing/python/definitionTagsTest.py for coding examples.

### smtk::attribute::Utilities Class
Added a class for utility methods.  The current ones include:

* associatableObjects - a method to return a set of persistent objects that can be assigned to a ReferenceItem
* checkUniquenessCondition - a method that removes resource::Components from a set based on the uniqueness constraint associated with a ComponentItem

### Changes to ReferenceItem
* Deprecated the following methods - note that the old methods are still available but will produce compiler warnings if used (C++14 feature)
  * objectValue replaced by value
  * setObjectValue replaced by setValue
  * appendObjectValues replaced by appendValues
  * setObjectValues replaced by setValues
* Added Methods
  * isValueValid - the validity of the item now factors in the state of the instance. This is needed to support Unique Constraints
  * removeInvalidValues() - will remove all values that are considered invalid - meaning that the resource of the associated object is loaded but the object no longer exists.
* **Behavior Changes**
  * removedValue() - will now remove a value regardless of what it's index is.  Previously, if the index was less than the NumberOfRequiredValues it would not be removed but simply unset.  Now, as long as the resulting number of values is greater than or equal to the number of required values, it will be removed.
  Else it will be unset.

### Custom attribute item and definition types
SMTK's attribute system now supports the registration of user-defined attribute items and definitions by overloading `smtk::attribute::CustomItem` and `smtk::attribute::CustomItemDefinition`, respectively. The registration of custom item definition types must occur before the attribute is serialized. Custom item definitions can be listed in plugins' Registrar implementations as follows:

```c++
void Registrar::registerTo(const smtk::attribute::ItemDefinitionManager::Ptr& manager)
{
  typedef std::tuple<CustomItemDefinition1, CustomItemDefinition2> CustomItemDefinitions;

  manager->registerDefinitions<CustomItemDefinitions>();
}
```

For an exmaple of its use, see smtk/attribute/testing/unitCustomItem.cxx.
