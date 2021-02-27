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

### Replacing attribute::Resource::updateCategories method
The new method is now called finalizeDefinitions since it now does more including updating advance level information.

### Changes to Category Modeling
#### Adding the concept of exclusion
Previously, you could provide a list of category names to an Attribute or Item Definition as well as Value Item Enums to indicate which categories should be "present" in order for the information to be considered relevant.  In addition, a combination mode indicating if any or all of the categories should be present.  This information is represented in attribute::Categories::Set.  Recently this has been expanded to include an additional set of category names that represent excluded categories.  As in the inclusion set of names, a combination mode indicating if any or all of the categories should not be present.  Finally, a top level combination mode indicating how to combine the results of the inclusion and exclusion sets.

See unitExclusionCategories test for an example of using this new functionality.

#### Turning off Category Filtering
With these new changes we can now model information that is relevant regardless of which categories are active.  For example Analysis Configuration Attributes which are used to indicate which categories should be active, should themselves not be filtered.  To do this, simple set the Item or Attribute's local categories to have a **combinationMode** set to **ANY**.  Since the inclusion and exclusion sets are initially empty which means they evaluate to false and true respectively, the passes method will always return true.

#### attribute::Categories::Set API Changes
* New API
  * combinationMode()/setCombinationMode(..) - Get/Set the how the sets of included and excluded categories are combined
  * inclusionMode()/setInclusionMode(..) - Get/Set the CombinationMode associated with the included categories.
  * exclusionMode()/setExclusionMode(..) - Get/Set the CombinationMode associated with the excluded categories.
  * includedCategoryNames() - Return the set of category names associated with the inclusion set.
  * excludedCategoryNames() - Return the set of category names associated with the exclusion set.
  * setInclusions(..) - Set the mode and category names of the inclusion set.
  * setExclusions(..) - Set the mode and category names of the exclusion set.
  * insertInclusion(..)/eraseInclusion(..) - add/remove category name to/from the inclusion set.
  * insertExclusion(..)/eraseExclusion(..) - add/remove category name to/from the exclusion set.
  * inclusionSize() - Returns the number of category names in the inclusion set.
  * exclusionSize() - Returns the number of category names in the exclusion set.
* Deprecated API
  * mode() -> inclusionMode()
  * setMode(..) -> setInclusionMode(..)
  * categoryNames() -> includedCategoryNames()
  * set(..) -> setInclusions(..)
  * insert(..) ->insertInclusion(..)
  * erase(..) -> eraseInclusion(..)
  * size() -> inclusionSize()

#### Changes to XML/JSON Formats
In the past the category names where added to their own XML element or JSON structure inside of the Attribute/Item Definition Block or inside of the ValueItem Enum Structure, but other aspects were stored else where.  Though this format is still supported for reading, the new format groups all of the Category Information together.  Here is an example of an Item Definition:

```xml
        <String Name="s0" Label="s0">
          <CategoryInfo Inherit="true" Combination="All">
            <Include Combination="All">
              <Cat>a</Cat>
              <Cat>b</Cat>
            </Include>
            <Exclude Combination="All">
              <Cat>c</Cat>
              <Cat>d</Cat>
            </Exclude>
          </CategoryInfo>
        </String>
```
This format is the same for Attribute Definitions and Enums with the exception of the Inherit attribute.  The following is the JSON equivalent:

```json
          "CategoryInfo": {
            "Combination": "All",
            "ExcludeCategories": [
              "c",
              "d"
            ],
            "ExclusionCombination": "All",
            "IncludeCategories": [
              "a",
              "b"
            ],
            "InclusionCombination": "All",
            "Inherit": true
          },
```

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

### Expression Support Changes
In previous releases of SMTK, if a ValueItem supported expressions, you could set either a value or expression for each "element" based on the Item's number of values.  This made it difficult to assign an expression to the entire Item which was determined to be the more important of the two UseCases.  The new API allows an expression to be set over the entire Item and not on an element by element basis.

#### Changes to ValueItem
* **Deleted Methods**
  * bool appendExpression(smtk::attribute::AttributePtr exp)
  * bool setExpression(std::size_t elementIndex, smtk::attribute::AttributePtr exp)
* **Modified Methods**
  * smtk::attribute::ComponentItemPtr expressionReference() const - no longer takes in an element index
  * bool isExpression() const  - no longer takes in an element index
  *  smtk::attribute::AttributePtr expression() const  - no longer takes in an element index

#### Changes to ValueItemDefinition
* **Modified Methods**
  * void buildExpressionItem(ValueItem* vitem) const  - no longer takes in an element index


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
#### Support for Active Children
Similar to Value Items, Reference Items can now have a set of active children items based on the
persistent object has been assigned to it.  Unlike, value items, where active children are based
on the enum value assigned, the active children for a reference item are based on a vector of
conditionals.  A conditional is composed of a resource query, component query and list of
item names.  A persistent object matches a conditional if it satisfies the conditional's
queries.  If the object is a Resource, only the resource query is tested.  If the object
is a Component, then it must satisfy both.

 In the case of a Component, the resource query can be an empty string, meaning the
 Component automatically satisfies the Resource part of the query.  Note that this should
 only be used if the Item can only be assigned Components that come from the same "type"
 of Resources.

 For examples see:

 * data/attribute/attribute_collection/refitem-categories.sbt
 * smtk/attribute/testing/cxx/unitReferenceItemChildrenTest.cxx

##### New API
 * ReferenceItemDefinition
   * std::size_t numberOfChildrenItemDefinitions()
   * const std::map\<std::string, smtk::attribute::ItemDefinitionPtr>& childrenItemDefinitions()
   * bool hasChildItemDefinition(const std::string& itemName)
   * bool addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
   * template <typename T> typename smtk::internal::shared_ptr_type\<T>::SharedPointerType addItemDefinition(const std::string& idName)
   * std::size_t addConditional(const std::string& resourceQuery, const std::string& componentQuery, const std::vector\<std::string>& itemNames)
   * std::size_t numberOfConditionals() const
   * const std::vector\<std::vector\<std::string> >& conditionalInformation() const
   * const std::vector\<std::string>& conditionalItems(std::size_t ith) const
   * const std::vector\<std::string>& resourceQueries() const
   * const std::vector\<std::string>& componentQueries() const
   * void buildChildrenItems(ReferenceItem* ritem)
   * std::size_t testConditionals(PersistentObjectPtr& objet) const
 * ReferenceItem
   * bool setObjectKey(std::size_t i, const Key& key, std::size_t conditional)
   * std::size_t numberOfChildrenItems() const
   * const std::map\<std::string, smtk::attribute::ItemPtr>& childrenItems()
   * std::size_t numberOfActiveChildrenItems() const
   * smtk::attribute::ItemPtr activeChildItem(std::size_t i) const
   * std::size_t currentConditional() const


#### Other Changes
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

### Changes to Item
* Added the concept of **ForceRequired**.  Some work-flows may need to force an optional item to be required base on external factors.  If
an item's ForceRequired is set then even if it's definition indicates it should be optional,  Item::isOptional will return false.  Note if the item's definition states it is required, setting ForceRequied is basically a no op.
    * New API
        * void Item::setForceRequired(bool)
        * bool Item::forceRequired()

### Changes to GroupItem
* New Methods
    * bool insertGroups(std::size_t ith, std::size_t num) - inserts num groups before ith element.  If inserting num groups would violate the item's maximum number of groups or if ith is greater than the original number of groups, then the item is left unchanged and false is returned. Append and Prepend methods have been modified to call this new method with num= 1 and ith = numberOfGroups or 0 respectively.

### Changes to ValueItem
* New Methods
    * std::string valueLabel(ith) - convenience method to access its Definition's valueLabel method.
    * bool setValueFromString(std::size_t ith, const std::string& val) - sets the ith value using a string which is converted to the appropriate data type.

### Custom attribute item and definition types
SMTK's attribute system now supports the registration of user-defined attribute items and definitions by overloading `smtk::attribute::CustomItem` and `smtk::attribute::CustomItemDefinition`, respectively. The registration of custom item definition types must occur before the attribute is serialized. Custom item definitions can be listed in plugins' Registrar implementations as follows:

```c++
void Registrar::registerTo(const smtk::attribute::ItemDefinitionManager::Ptr& manager)
{
  typedef std::tuple<CustomItemDefinition1, CustomItemDefinition2> CustomItemDefinitions;

  manager->registerDefinitions<CustomItemDefinitions>();
}
```

For an example of its use, see smtk/attribute/testing/unitCustomItem.cxx.

### Adding the Concept of Styles
Previously any customization for displaying an attribute needed to be explicitly defined within the  View Configuration specification.  Therefore, if the an attribute appeared in two different Views, then the customization needed to be explicitly copied in order for the attribute to be displayed the same way within the Views.  In addition, if an attribute was being created "in place" such as expression attributes being created when setting a Value Item's expression, there was no way to provide customization for the newly created attribute being displayed in the Attribute Editor Dialog.  The Style concept provides the means of defining global "Styles" for an attribute.  These styles can be overridden by the View Configuration.

In addition, Styles can be inherited. When asking the resource for a definition's style, if the name does not exist for that definition, the resource will check its base definition to see if the style exists for it.

#### Default Styles
A style with "" as its name is considered a default style;

#### Styles and Inheritance
There is no explicit checking to see if the item style information corresponds to items defined within the definition itself.  So a style could contain information that refers to items that exist in derived Definitions.
#### Using Styles in View
You can refer to a named style by adding Style="nameOfStyle" to your view configuration information.  If no Style is specified but a default style exists, the default style will be used.
### New API
  const smtk::view::Configuration::Component& findStyle(const smtk::attribute::DefinitionPtr& def,
    const std::string& styleName="") const;
  const std::map<std::string, smtk::view::Configuration::Component>& findStyles(const smtk::attribute::DefinitionPtr& def) const;
  const std::map<std::string, std::map<std::string, smtk::view::Configuration::Component> >& styles() const
* void addStyle(const std::string& definitionType, const smtk::view::Configuration::Component style) - adds or replaces a style for an attribute type.  the style component may have a Name Configuration Attribute that indicates the name of the style. If no name is found it is assumed to be the default style for the attribute definition type.
* const smtk::view::Configuration::Component& findStyle(const smtk::attribute::DefinitionPtr& def,
    const std::string& styleName="") const - returns the style associated with definition based on the style name.  **Note** If a style can not be found for the definition, its base definition is then searched.
* const std::map<std::string, smtk::view::Configuration::Component>& findStyles(const smtk::attribute::DefinitionPtr& def) const - returns all of the styles directly associated with an attribute definition.  **Note:**  That unlike findStyle, this method is not recursive.
* const std::map<std::string, std::map<std::string, smtk::view::Configuration::Component> >& styles() const - returns the map of styles contained in the resource.

### I/O and Python
Both XML and JSON have been updated to make use of Style Information.  Note that Version 4 XML files support Styles.  Python API has also been added.

The following is a default style example in XML:

```xml
  <!-- Style specifications -->
  <Styles>
    <Att Type="outputs">
      <Style>
        <ItemViews>
          <View Path="/output-times" ImportFromFile="true" LoadButtonText="Import from File"
            FileFormat="Double" BrowserTitle="Import from Double File"
            ValueSeparator="," CommentChar="#" FileExtensions="Time Files (*.txt *.csv *.dat)"/>
        </ItemViews>
      </Style>
    </Att>
  </Styles>
```
The following shows an example of using a named style:
```xml
  <!-- Style specifications -->
  <Styles>
    <Att Type="outputs">
      <Style Name="foo">
        <ItemViews>
          <View Path="/output-times" ImportFromFile="true" LoadButtonText="Import from File"
            FileFormat="Double" BrowserTitle="Import from Double File"
            ValueSeparator="," CommentChar="#" FileExtensions="Time Files (*.txt *.csv *.dat)"/>
        </ItemViews>
      </Style>
    </Att>
  </Styles>
<Views>
    <View Type="Instanced" Title="General">
      <InstancedAttributes>
        <Att Name="outputs-att" Type="outputs" Style="foo"/>
      </InstancedAttributes>
    </View>
</Views>
```
