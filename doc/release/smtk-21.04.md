# SMTK 21.04 Release Notes
This is the first release of SMTK that follows our new release naming convention where the major revision number  corresponds to the year of the release and the minor revision number corresponds to the month.

Note any non-backward compatible changes are in ***bold italics***. See also [SMTK 3.3 Release Notes](smtk-3.3.md) for previous changes.

## Changes to Common
### Updates to typeName() logic
The `smtk::common::typeName<>()` function's logic has been modified to no longer
check for the existence of a `virtual std::string typeName() const` method
associated with objects passed as the template parameter. The check was removed
to avoid the case where derived classes were inheriting their parent's typename,
resulting in unexpected behavior (instead of either a compile-time error or a
run-time exception).

### Updates to Observers
`smtk::common::Observers` functors are now automatically removed from the
`smtk::common::Observers` instance when their key goes out of scope. The original
functionality requiring that the functor be manually removed can be recovered by
calling `release()` on the functor's Key.

## Changes to SMTK's Resource System
### Removed `smtk::resource::Set`
`smtk::resource::Set` has been removed from SMTK. It's original purpose was to
hold a collection of resources for export. This functionality has been
superceded by Project.

### Added `smtk::resource::query::Manager`
`smtk::resource::query::Manager` is an interface for registering Query types.
SMTK Plugins can interact with `smtk::resource::query::Manager` by adding
methods similar to the following to their Registrar:

```
registerTo(const smtk::resource::query::Manager::Ptr& manager)
{
  manager->registerQuery<ApplicableResourceType, MyQuery>();
}

unregisterFrom(const smtk::resource::query::Manager::Ptr& manager)
{
  manager->unregisterQuery<MyQuery>();
}
```

Additionally, the `smtk_add_plugin()` call for the plugin should be extended
to include `smtk::resource::query::Manager` in its list of managers.
Upon registration, all appropriate resources associated with the same
resource manager as the `smtk::resource::query::Manager` will be able to
construct instances of the newly registered Query.

### Other Changes
* queryOperation now returns a functor that takes in a const smtk::resource::Component& instead of a const smtk::resource::ConstComponentPtr&

* Resources now have Query functors. Queries are functors designed to incrementally augment the API of a Resource at runtime via registration.

* Derived Resources can now be defined entirely in Python.

## Changes to Resource Manager
### Making Resource Manger Thread-Safe
Instances of `smtk::resource::Manager` no longer provide direct access to the `smtk::resource::Container`
(a boost multi-index container) in order to provide thread-safe insertion, traversal, and removal of
resources.

Instead of calling `smtk::resource::Manager::resources()`, you should now use one of the following methods:

+ One of the existing `find()` methods to obtain resources by location, or type-index.
+ One of the existing `get()` methods to obtain resources by ID.
+ `void smtk::resource::Manager::visit(const Visitor& visitor) const` — to iterate over all resources
  held by the manager. Note that access to the resource manager from inside the visitor is limited to
  read-only methods. Methods that modify the set of resources held by the manager may not be called until
  after `visit()` exits. The proper strategy for inserting or removing resources is to have the visitor
  capture a list of changes queued during visitation and perform modifications once visitation is done.
+ `bool smtk::resource::Manager::empty() const` — to verify the manager has no resources.
+ `std::size_t smtk::resource::Manager::size() const` — to get a count of resources (but be aware this cannot
  be safely used for iteration of any sort since the size may be changed by other threads after the method
  returns. This method is intended mainly as a convenience for unit tests.

Note that registering/unregistering resource types (i.e., `smtk::resource::Metadata`) is still not threadsafe.


## Attribute Resource Changes
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

#### Introducing Categories and Categories::Sets
##### Original Design

Attribute Category Information was originally stored as a set of strings (representing the names of the categories) and a combination mode.  When inheriting the information, only the set of strings were passed to parent items, attributes and possibly children items (based on their inherit category mode).

##### The Problem
Without the combination mode, the inherited categories allowed attributes and items to be displayed with no internal children.  Consider the following example:
* Attribute Definition A has no local categories but contains 2 Items
* Item i1 has local categories (c1, c2) with combination mode ALL
* Item i2 has local categories (c3, c4) with combination mode ALL

In the original implementation, A inherited categories (c1, c2, c3, c4) with mode ANY. If the GUI is displaying information associated with c1, any attributes of type A were displayed but will have no content.

##### New Design
Replace the std::set\<std:string> , mode representation of category information with a class called smtk::attribute::Categories that provides the following functionality:
* Represents sets of categories along with the combination mode
* Provides a mechanism of combining 2 Categories instances
* Provides a passes method that takes in a set of category strings and returns true if the Categories instance would be "include" in that set.

```c++
namespace smtk {
namespace attribute {
class Categories
{
   class Set
   {
      public:
         enum class CombinationMode
            { Any, All };
         CombinationMode mode() const { return m_mode;}
         void setMode(const Set::CombinationMode& newMode);
         const std::set<std::string>& values { return m_set;}
         void set(const std::set<std::string>& values, CombinationMode mode);
         bool empty() const { return m_categoryNames.empty(); }
         std::size_t size() const { return m_categoryNames.size(); }
         bool operator<(const Set& rhs) const;
      private:
         Combination m_mode;
         std::set<std::string> m_set;
   };
public:
   bool passes(const std::set<std::string>& cats) const;
   void append(const Set& set) { m_sets.insert(set); }
   void reset() { m_sets.clear(); }
   const std::set<Set>& sets() const { return m_sets;}
   std::size_t size() const { return m_sets.size(); }
private:
   std::set<Set> m_sets;
};
};
};
```
##### API Changes
* Attribute
  * New Methods
     * categories() - returns a reference to the Categories object associated with the Attribute
  * Removed Methods
     * isMemberOf(...) -> replace with categories().passes(...)
* Definition
  * Changed Methods
     * categories() - now returns a reference to the Categories object associated with the Definition
     * localCategories() - now returns a reference to the Categories::Set object associated with the Definition's local categories
     * applyCategories - method now takes in Categories and a set of strings
  * Removed Methods
     * isMemberOf(...) -> replace with categories().passes(...)
     * numberOfCategories() - replaced with categories().size()
     * addLocalCategories(...) - replaced with localCategories().insert(...)
     * removeLocalCategories(...) - replaced with localCategories().erase(...)
* ItemDefinition
  * Changed Methods
     * categories() - now returns a reference to the Categories object associated with the ItemDefinition
     * localCategories() - now returns a reference to the Categories::Set object associated with the ItemDefinition's local categories
     * applyCategories - method now takes in Categories and a set of strings
   * CategoryCheckMode Enums removed - replaced by Categories::Set::CombinationMode
  * Removed Methods
     * isMemberOf(...) -> replace with categories().passes(...)
     * numberOfCategories() - replaced with categories().size()
     * addLocalCategories(...) - replaced with localCategories().insert(...)
     * removeLocalCategories(...) - replaced with localCategories().erase(...)
     * setCategoryCheckMode(...) - replaced with localCategories().setMode(...)
     * categoryCheckMode() - replaced with localCategories().mode()
* Item
  * New Methods
     * categories() - returns a reference to the Categories object associated with the Item
  * Removed Methods
     * isMemberOf(...) -> replace with categories().passes(...)
     * passCategoryCheck(...) - replaced with categories().passes(...)
* ValueItemDefinition
   * Changed Methods
       * setEnumCategories - now takes in Categories::Set instead of a set of strings
       * enumCategories - now returns a const reference to a Categories::Set instead of a set of strings

##### Notes
###### Enum Category Constraints
Category constraints placed on enums are inherited by the ValueItem but not by the ValueItem's children.


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

#### Added "Rejects" filter for ReferenceItemDefinition
ReferenceItemDefinition now has a "Rejects" list. While a nonempty "Accepts" list
requires an object to be listed (either directly or indirectly via inheritance)
in order to be valid, any object that is listed (again, directly or indirectly)
in the "Rejects" list is invalid. If an item is listed in both the "Accepts" and
"Rejects" filters (directly or indirectly), the "Rejects" behavior takes
precedence and the object is invalid.


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

### Changes to GroupItem and GroupItemDefinition
#### Conditional Groups
This allows a group (and it's subgroups) to be treated a set of options with the additional requirement that there should be at least minNumberOfChoices but no more than maxNumberOfChoices of relevant items enabled.  This condition will effect the item's validity calculation. See [here](https://discourse.kitware.com/t/expanding-groupitem-functionality/622) for more information.
##### New API to GroupItemDefinition
* setIsConditional(bool) - sets the isConditional property of the definition.  If true then the definition is treated as a collection of conditions.  All of its items will be considered optional and an added validity constraint will be imposed on the number of relevant enabled items. (false by default)
* bool isConditional() - returns the current value of the isConditional property.
* setMinNumberOfChoices(unsigned int) - indicates the min number of relevant items that should be enabled.  (0 by default)
* unsigned int minNumberOfChoices() - returns the min number of relevant items that should be enabled.
* setMaxNumberOfChoices(unsigned int) - indicates the max number of relevant items that should be enabled.  If set to 0, there is no max number. (0 by default)
* unsigned int maxNumberOfChoices() - returns the max number of relevant items that should be enabled.

The following is a SBT file showing an example.

```xml
        <Group Name="opt1" Label="Pick At Least 2"
          IsConditional="true" MinNumberOfChoices="2" MaxNumberOfChoices="0">
          <ItemDefinitions>
            <String Name="opt1"/>
            <Int Name="opt2"/>
            <Double Name="opt3"/>
          </ItemDefinitions>
        </Group>

```
See data/attribute/attribute_collection/choiceGroupExample.sbt and smtk/attribute/testing/unitConditionalGroup.cxx for a more complete example.
##### New API to GroupItem
* bool isConditionalsSatisfied() - indicates if the group item satisfies it's conditional constraint.  If the item's definition has it's isConditional property set to false, this method will always return true.
* bool isConditional() - returns the current value of the isConditional property.
* setMinNumberOfChoices(unsigned int) - indicates the min number of relevant items that should be enabled.  (initially set to the definition's value)
* unsigned int minNumberOfChoices() - returns the min number of relevant items that should be enabled.
* setMaxNumberOfChoices(unsigned int) - indicates the max number of relevant items that should be enabled.  If set to 0, there is no max number. (initially set to the definition's value)
* unsigned int maxNumberOfChoices() - returns the max number of relevant items that should be enabled.

#### Other New Methods

* `bool insertGroups(std::size_t ith, std::size_t num)` - inserts num groups before ith element.  If inserting num groups would violate the item's maximum number of groups or if ith is greater than the original number of groups, then the item is left unchanged and false is returned. Append and Prepend methods have been modified to call this new method with num= 1 and ith = numberOfGroups or 0 respectively.

### Changes to ValueItem
* New Methods
    * `std::string valueLabel(ith)` - convenience method to access its Definition's valueLabel method.
    * `bool setValueFromString(std::size_t ith, const std::string& val)` - sets the ith value using a string which is converted to the appropriate data type.

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

### Active Categories

The concept of active categories (categories that represent the active concepts of the work flow) have been added to the SMTK attribute system.
They are a set of category strings that are assigned to the Attribute Resource.  The new methods include:

* setActiveCategoriesEnabled - enables/disables active categories (default is false)
* activeCategoriesEnabled - returns the current active categories enable mode
* setActiveCategories - sets the set of active categories
* activeCategories - returns the current set of active categories
* passActiveCategoryCheck - methods to test Categories and Categories::Sets based on the Resource's active categories settings

#### Other Changes
##### isValid Methods
Both Attribute and Item isValid() methods that use to take in no arguments now take in a bool to indicate if their Resource's active categories state should be used to determine validity.  The parameter is by default set to true.  Note that this does not change previous functionality since the Resource's default is disabled (aka false).

#### isRelevant Methods
Both Attribute and Item now have an isRelevant method that returns true if they are relevant based on the Resource's active category state.  If the Resource has active categories disabled then these methods will always return true, else the method will test their categories based on the resource's active set.

#### ReferenceItemDefinition
Added a new aspect called **enforcesCategories**.  In the case where a Reference Item is pointing to Attributes, this property means that the Item is valid iff the Attribute its pointing to is considered relevant (meaning it passes the current set of active categories).

#### I/O Changes for both JSON and XML
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

#### UI Changes
* All Category related methods have been removed from the UI Manager and now use the active category  mechanism
* qtBaseAttribute Category tests have been changed to take in a shared pointer to an Item instead of a reference to an ItemDefinition. - This was needed since ItemDefinitions do not have a way of getting access to the Resource.

##### qtReferenceItemEditor Changes
* qtReferenceItemEditor no longer uses "UseCategories" option in its ItemView.
* Now enforces the new **enforcesCategories** property
* When the item's value becomes invalid due to active category changes, this will now properly update the UI.

### Added thread-safe API for manipulating links
smtk::resource::Resource::Links and smtk::resource::Component::Links do not have a thread-safe API. Since resources and components are designed to be manipulated within an operation, concurrency issues are designed to be handled at a higher scope. smtk::attribute is an exception to this, and attributes are often manipulated directly. As a result, an API has been added to smtk::attribute::Resource and smtk::attribute::Attribute to manipulate link information in a thread-safe manner.


### Update to qtAttributePreview example


The command line arguments were revised and expanded.
New features include:
* Model files can be loaded in addition to the attribute
  template. Model files are loaded using VTK session.
* The SMTK Operation template can be preloaded, so that
  operation templates can be loaded and displayed.

The new usage output is:

```
Usage: ./bin/qtAttributePreview [options] attribute_filename
Load attribute template and display editor panel

Options:
  -h, --help                Displays this help.
  -m, --model-file <path>   Model file (using vtk session)
  -o, --output-file <path>  Output attribute file (.sbi)
  -p, --preload-operation   Preload smtk operation template
  -v, --view-name <string>  Specific View to display

Arguments:
  attribute_filename        Attribute file (.sbt)
```

### ItemDefinitionManager: remove requirement for resource manager

Originally, ItemDefinitionManager acted on resources indirectly through
the resource manager. The new design can still associate to a resource
manager and augment its attribute resources, but it is also functional as
a standalone manager and can add custom item definitions to attribute
resources directly. This change removes the requirement for attribute's
Read and Import operations to construct attribute resources using the
resource manager, so attribute resources can now be populated prior to
being added to the resource manager.

### Introduce custom association rules

The ability to append custom rules that determine whether an object can be
associated/dissociated to/from an attribute has been added. It is used in
the following way:
1. A custom rule type that inherits from `smtk::attribute::Rule` is either
registered to an attribute resource directly via its Rule factories
(e.g.
```
resource->associationRules().associationRuleFactory().registerType<FooRule>();
resource->associationRules().associationRuleFactory().addAlias<FooRule>("Foo");
```)
or indirectly using the AssociationRuleManager (e.g.
```
void Registrar::registerTo(const smtk::attribute::AssociationRuleManager::Ptr& manager)
{
  manager->registerAssociationRule<FooRule>("Foo");
}
```).
2. An instance of this custom rule type, identified by its alias, can then
be defined in the v4 XML description of an attribute resource:
```
  <AssociationRules>
    <Foo Name="myFoo">
      ...
    </Foo>
  </AssociationRules>
  <DissociationRules>
    ...
  </DissociationRules>
  <Definitions>
    ...
  </Definitions>
</SMTK_AttributeResource>
```
3. Finally, An attribute definition can be associated with the custom
rules:
```
<AttDef Type="att" BaseType="">
  <AssociationRule Name="myFoo"/>
  ...
</AttDef>

```

### Introduce PythonRule

As an example of a custom association/dissociation rule, a PythonRule
has been introduced that accepts a Python snippet describing the rule:
```
<PythonRule Type="smtk::attribute::PythonRule" Name="myPythonRule"><![CDATA[
def canBeAssociated(attribute, object):
    print("can %s be associated to %s? Is it a mesh component?" % \
         ( attribute, object ) )
    import smtk.mesh
    meshComponent = smtk.mesh.Component.CastTo(object)
    return meshComponent != None
  ]]></PythonRule>
```

PythonRule instances expect a Python function that accepts "attribute"
and "object" input parameters to determine whether an object can be
associated/dissociated to/from an attribute. Additionally, external Python
source files describing different modules can be listed using the <SourceFiles>
XML tag. For an example of its use, see `unitAssociationRulesTest.cxx`.

### Expression Evaluators
Evaluators are a major new feature for parsing user-provided expressions as item values. See the user's guide for Attribute for details.


## Model Resource Changes
### Geometric interpolation for attribute values on model entities

Routines have been added that parse a geometry with associated attributes and
interpolate the attribute values at user-defined sample points in 3-space.

### Other Changes
+ Fixed bounding box method on EntityRef to examine
  both property data (old smtk::model::Tessellation style)
  and geometry data (new smtk::geometry::Geometry style).

## Mesh Resource Changes
### Preliminary support for higher order cells
Preliminary support for higher order cells has been added to SMTK's mesh
library. Currently, higher order meshes can be read from .vtu files, written to
.vtu files and visualized. All nonlinear cells are currently cast as Lagrange
cells whose order is dictated by the number of points that comprise the cell.

### Geometry

The mesh subsystem now uses the new `smtk::geometry::Geometry` provider to supply
data for rendering.

The default mode for writing a mesh has been changed to archive the mesh description and the mesh
file into a single file. The `smtk::mesh::WriteResource` operation has a flag that can revert this
functionality back to its original logic of writing multiple files.


## Changes to SMTK Operations
### Removal of MarkModified operation
The MarkModified operation has been removed since it was considered redundant.  The attribute Signal operation should be used instead.

### operation::Manager: provide access to common::Managers

Operations frequently require access to other managers. This was
originally accomplished using inheritance to subset operations that
needed access to a manager type, and then modifying their create methods
to include access to the manager at runtime. With the introduction of a more
modular means to add managers to SMTK, this approach proved unscalable to
new manager types.

The new approach provides access to all managers through the operation
manager, if it is available. While scalable, the trade-off is the
requirement of an instance of smtk::common::Managers that holds available
managers.

### Behavior for intercepting application close requests when operations are running

A behavior has been added that tracks when operations are active. When a close
event is received from the main window, a modal dialog confirming the
application's close is presented if there are active operations.

#### User-facing changes

When an operation is running, applications built on the ParaView framework will
now warn and provide an option to cancel close events.

### smtk\_operation\_xml() replaced with smtk\_encode\_file()

When operations add a .sbt xml file to specify the interface of the operation,
it was wrapped into a header file by the cmake function
`smtk_operation_xml()`.  Update operations to use `smtk_encode_file()`,
because it will generate the file at build time instead of cmake configure
time, and the build tool (like `ninja`) will pick up on changes to the .sbt
file and rebuild.

### Other Changes
* There is now a group to hold component-deletion operations (named DeleterGroup).

## I/O Changes
### Added Preliminary Support for Writing Attribute Libraries
The AttributeWriter can now be used to save a subset of Attributes based on a collection of Attribute Definitions.  This is the first step in supporting the creation of attribute libraries.  The new functionality include:

* includeAdvanceLevels - method to indicate if the AdvanceLevels section should be included in the saving process
* includeAnalyses - method to indicate if the Analysis section should be included in the saving process
* setIncludedDefinitions - Restricts the types of attribute instances written out to those derived from a specified list.  If the list is empty, then all attributes will be saved. Any redundant definitions (definitions that can be derived from others in the list) are removed.
* includedDefinitions - Returns the list of definitions to be used to filter attribute instances.  If empty then there is no attribute filtering
* treatAsLibrary - A convenience  method for creating a library. Write/WriteAsContents will produce a library like XML file containing only attribute instances that are based on the provided list of definitions. If the list is empty, then all attributes will be saved. This method will, by default, not include Analyses, AdvanceLevels, Definitions or View sections - these sections can be included by calling enabling them after calling this method.

### Added Support for Attribute Styles
This is supported in Version 4 XML Files.
**Note** Current Limitation: If writing out XML files to replicate the original include file structure, all of the style information will be saved in the top most file.  The reason for this is Style information read from included files can be over written in the more top-level files and there is currently no way to track this.

### Added attributeUtils
This is a place to store I/O utilities for attribute related information.

#### importFromCSV
 bool importFromCSV(smtk::attribute::GroupItem& item, const std::string& filename,
    Logger& logger, bool appendToGroup = false, const std::string& sep = ",", const std::string& comment = "")

 This function imports information from a CSV formatted file into a GroupItem that has the following characteristics:
 * It is extensible
 * All of its children are ValueItems
 * All of its children are not optional or extensible

 The function can ether overwrite the item's current groups or append to them.  Any input line that does not have the proper number of values, based on the item's structure, will be skipped and recorded in the logger.  Errors and warnings are returned through the logger.

#### importFromDoubleFile
 bool importFromDoubleFile(smtk::attribute::GroupItem& item, const std::string& filename,
  Logger& logger, bool appendToGroup = false, const std::string& optionalSep = ",", const std::string& comment = "")

 This function imports information from a file containing only doubles into a GroupItem that has the following characteristics:
 * It is extensible
 * All of its children are DoubleItems
 * All of its children are not optional or extensible

 The function can ether overwrite the item's current groups or append to them.  Any input line that has less than the proper number of values, based on the item's structure, will be skipped and recorded in the logger.  Errors and warnings are returned through the logger.

### Adding Support for Defining Analysis Configurations
Previously, in order to define an analysis configuration, the author either needed to "guess" the attribute definition structure of the analysis and then construct an attribute representation for the configuration by hand or run the template and create an analysis configuration using ModelBuilder, save out the file in XML and then copy the configuration back into the template and use it as a guide.
This enhancement now allows the template author to define analysis configurations simply using the analysis structure defined in the file.

For Example, consider the following analysis specifications:

```xml
  <Analyses Exclusive="true">
    <Analysis Type="A">
      <Cat>A</Cat>
    </Analysis>
    <Analysis Type="B">
      <Cat>B</Cat>
    </Analysis>
    <Analysis Type="C" Exclusive="true">
      <Cat>C</Cat>
    </Analysis>
    <Analysis Type="B-D" BaseType="B">
      <Cat>D</Cat>
    </Analysis>
    <Analysis Type="B-E" BaseType="B">
      <Cat>E</Cat>
    </Analysis>
    <Analysis Type="C-D" BaseType="C">
      <Cat>D</Cat>
    </Analysis>
    <Analysis Type="C-E"  Exclusive="true" BaseType="C">
      <Cat>E</Cat>
    </Analysis>
    <Analysis Type="C-E-D" BaseType="C-E">
      <Cat>D</Cat>
    </Analysis>
    <Analysis Type="C-E-F" BaseType="C-E">
      <Cat>F</Cat>
    </Analysis>
  </Analyses>
```
Let us assume we want to create the following configurations:

* Test A - set the top-level to A
* Test B - set the top-level to B
* Test B-D - set the top-level to B and turn on D
* Test C-D - set the top-level to C and select D
* Test C-E-F - set the top-level to C and select E and the select F

It should also catch the following invalid configurations:

* Test C - can't simply set the top-level to C since C itself has exclusive analyses
* Test C-E similarity to the previous example since E also has exclusive analyses

The following would create 5 valid configurations and prevent the 2 invalid ones from being constructed:

```xml
  <Configurations AnalysisAttributeType="Analysis">
    <Config Name="Test A" AdvanceReadLevel="5">
      <Analysis Type="A"/>
    </Config>
    <Config Name="Test B" AdvanceWriteLevel="10">
      <Analysis Type="B"/>
    </Config>
    <Config Name="Test B-D">
      <Analysis Type="B">
        <Analysis Type="B-D"/>
      </Analysis>
    </Config>
    <Config Name="Test C">
      <Analysis Type="C"/>
    </Config>
    <Config Name="Test C-D">
      <Analysis Type="C">
        <Analysis Type="C-D"/>
      </Analysis>
    </Config>
    <Config Name="Test C-E">
      <Analysis Type="C">
        <Analysis Type="C-E"/>
      </Analysis>
    </Config>
    <Config Name="Test C-E-F">
      <Analysis Type="C">
        <Analysis Type="C-E">
          <Analysis Type="C-E-F"/>
        </Analysis>
      </Analysis>
    </Config>
  </Configurations>
```

The file  data/attribute/attribute_collection/analysisConfigTest.sbt models the above example and unitAnalysisConfiguration.cxx is the unit test for verifying the result.

### Added Support for Item Definition Blocks (XML Format Only)
Item Definition Blocks allows the reuse of a group of Item Definitions in different Attribute Definitions.  Providing a "hasA" relationship as opposed to the currently supported "isA". These blocks can then be referenced in the "ItemDefinitions" nodes of Attribute or Group Item Definitions or in the "ChildrenDefinitions" nodes for Reference or Value Item Definitions.  Blocks themselves can reference other blocks.  But care must be taken not to form a recursive relationship.  In the parser detects such a pattern it will report an error.

When referencing a Block, the items will be inserted relative to where the Block is being referenced.

Note that category constraints are inherited as usual and that Blocks can call other blocks.  Here is an example of an Item Block:

```xml
  <ItemBlocks>
    <Block Name="B1">
      <ItemDefinitions>
        <String Name="s1">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i1"/>
      </ItemDefinitions>
    </Block>
  </ItemBlocks>

  <Definitions>
    <AttDef Type="Type1">
      <Categories>
        <Cat>Fluid Flow</Cat>
      </Categories>
      <ItemDefinitions>
        <Double Name="foo"/>
        <Block Name="B1"/>
        <String Name="bar"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Type2">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="B1"/>
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

```
See data/attribute/attribute_collection/ItemBlockTest.sbt and smtk/attribute/testing/cxx/unitItemBlocks.cxx for examples.


## Session Changes
### VTK session

The VTK session now uses the new `smtk::geometry::Geometry` provider
to supply data for rendering without copying. This is a breaking
change and introduces a new dependency on vtkSMTKSourceExt for both
its Geometry class (which `smtk::session::vtk::Geometry` inherits) and
the vtkResourceMultiBlockSource so that UUIDs can be stored uniformly
by the session and the VTK backend.

Sessions which inherit the VTK session will need to be updated:

+ All use of `smtk::session::vtk::Session::SMTK_UUID_KEY()` should be
  eliminated in favor of calls to `GetDataObjectUUID` or
  `SetDataObjectUUID` in vtkResourceMultiBlockSource.
+ Instead of calling `smtk::session::vtk::Session::addTessellation()` on
  a model entity, you should call
  `smtk::operation::MarkGeometry(resource).markModified()` on the entity
  instead.

### Mesh Session
The mesh session now uses the new `smtk::geometry::Geometry` provider to supply
data for rendering.

The default mode for writing a mesh session model has been changed to archive the model
description and the mesh file into a single file. The `smtk::session::mesh::Write` operation
has a flag that can revert this functionality back to its original logic of writing multiple files.

### OpenCASCADE Session

SMTK now includes a minimal OpenCASCADE session; it can import
OpenCASCADE's native `.brep` files in addition to STEP and IGES
files. This session takes advantage of the new `smtk::graph::Resource`.


## Changes to the View System and Qt Extensions

### VTK-only application and threads

An smtk application that uses Qt widgets, and only VTK and not Paraview can make two choices reguarding operations and threads:
* Run all operations on a single thread.
   * Set the cmake variable `SMTK_ENABLE_OPERATION_THREADS` to `OFF` when compiling SMTK, to disable threading.
* Forward signals from operation observers to the main thread:
   * After creating and populating `smtk::common::Managers` with the operation and resource manager, use
     `qtSMTKCallObserversOnMainThreadBehavior` to do this forwarding.

```
#include "smtk/extension/qt/qtSMTKCallObserversOnMainThreadBehavior.h"

auto *observersOnThread = qtSMTKCallObserversOnMainThreadBehavior::instance(win);
observersOnThread->forceObserversToBeCalledOnMainThread(m_managers);

```

Otherwise if operation observers try to change Qt state, unexpected behavior/crashes will result.
### View factory redesign

- As part of a redesign of the view subsystem, view::View is being renamed to view::Configuration since its purpose is to hold a specification of a view rather than the view itself. In future releases, view::View will be reintroduced as a class responsible for presenting a view (in a way independent of any GUI library).
- The view::Manager class has taken over responsibility for creating qtBaseView-derived classes from qtUIManager, and uses the Registrar pattern.
- The cmake function `smtk_plugin_add_ui_view()` has been removed, and all qtBaseView-derived classes have been added to a Registrar. External plugins will need to make a similar change.

### View IconFactory Renamed to Object Icons
+ Renamed `smtk::view::IconFactory` to `smtk::view::ObjectIcons` (since it
  is specifically an icon factory for depicting the types of PersistentObject.
  You will need to replace `#include "smtk/view/IconFactory.h"` with the
  new header and calls to `smtk::view::Manager::iconFactory()` with calls
  to `smtk::view::Manager::objectIcons()`.
+ Added a new `smtk::view::OperationIcons` class (an instance of which is
  held by the `smtk::view::Manager`) that produces icons for operations.

### Improving Observer Stability
It was discovered that passing the **this** pointer into an Observer's lambda expression in a Qt-based class can cause a crash if the object is deleted while there is an event still in the Qt Event loop.  The solution is to use a QPointer instead so that it can be tested for nullptr.

### Adding Observers to Views
* Observation for attribute creation, modification, and expungement have been added to qtAttributeView.
* Observation for attribute modification has been added to qtInstanceView

### Changes to Processing DiscreteItems
#### Added "Please Select" option
In addition to showing all of the possible enum values, the qtDiscreteValueEditor will include a "Please Select" option.  This is used to show that the item is not set and can be used to unset the item.

#### Processing Category and Advance Level
Enums can now be filtered out based on the category and advance level information explicitly assigned to the enum.  If the item's current value is not considered "valid" based on the current category/advance level settings, it is added to the list but is colored red to indicate that it is not considered "valid".

### Added a FixedWidth method to qtItem
A qtItem can now indicate if its width is fixed.  This is used by containers such as
qtGroupItem's Table to determine proper resize behavior

### Changes to qtResourceBrowser

* Now inherits qtBaseView instead of QWidget, to allow configuration via a ViewInfo.
* A default .json config specifies the default PhraseModel and SubphraseGenerator types to use.
* The smtk::view::Manager class can dynamically construct PhraseModels and SubphraseGenerators based on typename.
Example:

```c++
  // get the default config.
  nlohmann::json j = nlohmann::json::parse(pqSMTKResourceBrowser::getJSONConfiguration());
  smtk::view::ViewPtr config = j[0];
  // modify the PhraseModel type
  config->details().child(0).setAttribute("Type", "smtk::view::ComponentPhraseModel");
  // modify the SubphraseGenerator type
  config->details().child(0).child(0).setAttribute("Type", "smtk::view::TwoLevelSubphraseGenerator");
```

### Changes to Icons
#### Replaced Resource and Component icons with SVG representations
Icons for Resources and Components are now represented by SVG images, rather than PNG. The use of vectorized images makes icons scale with high retina desplays, and the programmatic instantation of these images allows for true color representations.

#### Added registration system for Resource and Component icons
Consuming applications can now register icon sets for Resources and Components, providing for more customization options and the use of icons that more closely reflect the entities they represent.
### Changes to qtSelectorView
* Now properly works with Analysis categories
* The view is considered empty if the selector item does not pass its category checks

### qtBaseView and qtBaseAttributeView Changes
* Made the following methods const
  * displayItem
  * isItemWriteable
  * advanceLevel
  * categoryEnabled
  * currentCategory

### qtAttribute Changes
* Added a removeItems methods - this removes all of the qtItems from the qtAttribute and allows createBasicLayout to be called again
* Added an option to createWidget that will allow a widget to be created even if the attribute's items are filtered out by categories and/or advanced level filtering.
* qtAttributeInternal now stores a QPointer  to a qtBaseAttributeView instead of a qtBaseView - this is more conceptually consistent and eliminated the need for dynamic casts.

### Changes to qtBaseView
* Added a virtual isValid method to indicate if a view is valid
* Added a modified signal so that qtBaseViews can indicate they have been changed

### Changes to qtBaseAttributeView
* CategoryTest was changed to take in a const attribute::ItemDefinitionPtr& instead of an attribute::ItemPtr since only the definition is needed.  This also eliminated the construction/destruction of a shared pointer.
* Added displayItemDefinition method which is similar to displayItem.
* isItemWriteable now takes in a const attribute::ItemPtr & instead of a attribute::ItemPtr which eliminates  the construction/destruction of a shared pointer.
* advaceLevelTest now takes in a const attribute::ItemPtr & instead of a attribute::ItemPtr which eliminates  the construction/destruction of a shared pointer.

### qtInstancedView Changes
* Added support for applying Configuration Styles stored in the attribute resource.  If an attribute does not have style information defined in the View, the View will check with the UIManager. **Note:** this process will also take into consideration the definitions that attribute's definition is based on.


### qtAttributeView Changes
* There is now a splitter between the attribute editing area and the association information area.
* Removed the old view by property mechanism to help simplify the code
* qtAttributeView and qtAssociatioView now have virtual methods to create their association widgets that can get overridden - in the future they should use a widget factor to fetch it so you wouldn't have to create a new class to use a different association widget
* XML Option RequireAllAssociated="true" will now display the qtAssociationWidget even if no attributes exists and display a warning if there are persistent objects that match the definition requirements but are not associated to any attribute.
* Added XML Option DisableNameField="true" that indicates that the attribute's name should not be changed.
* The top widget is now based on a qtTableView instead of a qtTableWidget and provides searching capabilities.
* Added the ability to search attributes by name (both case sensitive and insensitive) - the search box's visibility can be controlled in the View's configuration using **DisplaySearchBox** as in the following example:

```xml
    <View Type="Attribute" Title="testDef" Label="Atts to be Associated"
      DisplaySearchBox="false">
      <AttributeTypes>
        <Att Type="testDef" />
      </AttributeTypes>
    </View>
```
* Added the ability to set the string in the search box when the user has entered no text.  This can be set by using **SearchBoxText** as in the following example:

```xml
    <View Type="Attribute" Title="A" Label="A Atts"
      SearchBoxText="Search by name...">
      <AttributeTypes>
        <Att Type="A" />
      </AttributeTypes>
    </View>
```
* Added the following methods to set/get the View's modes
  * Mode for displaying Association Wdiget
     * void setHideAssociations(bool mode);
     * bool hideAssociations() const;
  * Mode for requiring all associatable object must be associated with an attribute
     * void setRequireAllAssociated(bool mode);
     * bool requireAllAssociated() const;
  *  Mode to indicate that attribute names are not allowed to be changed.
     * void setAttributeNamesConstant(bool mode);
     * bool attributeNamesConstant() const;
  * Added the ability to force the attribute name to be limited by a regular expression using **AttributeNameRegex**. ***NOTE: Currently the expression string should end with an '*'.***  If it does not the user will be able to initially enter invalid characters the first time but the widget will not allow the change to be saved.  Subsequent edits will work as expected.  This seems to be a bug in Qt.

```xml
    <View Type="Attribute" Title="A" Label="A Atts"
      SearchBoxText="Search by name..." AttributeNameRegex="[a-zA-Z_][a-zA-Z0-9\-]*" TopLevel="true">
      <AttributeTypes>
        <Att Type="A" />
      </AttributeTypes>
    </View>
```
* Added support for applying Configuration Styles stored in the attribute resource.  If an attribute does not have style information defined in the View, the View will check with the UIManager. **Note:** this process will also take into consideration the definitions that attribute's definition is based on.

#### Changes to qtGroupView
* GroupBox Style icon has been changed from a check box to a closed/expand icon pair.  This change reduces confusion between optional items and viewing control widgets.
* Tabbed Group Views now show indicate invalid children views in their tabs using the alert icon

### API Changes
These changes were made to help simplify/cleanup the qtView infrastructure.  There were several places where onShowCategory() was being called in order to update the UI.  This resulted in confusion as to the role of the method.  In many cases these calls have been replaced with updateUI.

* **qtBaseView::updateViewUI - has been removed.** It was not being used.
* **qtBaseAttributeView::updateAttributeData - has been removed.** This method's role was to update the attribute content of a View.  You should now call updateUI() instead.
* qtBaseAttributeView no longer overrides updateUI()

### Tracking Changes in Analysis Configuration Attributes
Attributes that are deleted, created, or modified are now checked to see if they represent Analysis Configurations.  The configuration combobox is then updated appropriately.

### qtUIManager Changes
* There is now a method to return the size of a string based on the font being used
* Added the following methods that take into consideration the current text color (which changes based on the system’s theme):
    * correctedInvalidColor()
    * correctedDefaultColor()
    * correctedNormalColor()
* Added the ability to access attribute style information.  Currently this simply calls the Style API of the attribute resource.
* The following methods have been removed due to the support of Active Categories
    * passAttributeCategoryCheck(smtk::attribute::ConstDefinitionPtr AttDef)
        * Replaced by Attribute::isRelevant() or attribute::Resource::passActiveCategoryCheck()
    * passItemCategoryCheck(smtk::attribute::ConstItemDefinitionPtr ItemDef)
        * Replaced by Item::isRelevant() or attribute::Resource::passActiveCategoryCheck()
    *  passCategoryCheck(const smtk::attribute::Categories::Set& categories)
        * Replaced by attribute::Resource::passActiveCategoryCheck()
    * disableCategoryChecks
        * Replaced by attribute::Resource::setActiveCategoriesEnabled(false)
    * enableCategoryChecks
        * Replaced by attribute::Resource::setActiveCategoriesEnabled(true)
    * setTopLevelCategories(const std::set<std::string>& categories)
        * No longer needed
    * checkAttributeValidity(const smtk::attribute::Attribute* att)
        * Replaced by Attribute::isValid()

### qtItem Changes
* Added a m_markForDeletion property that gets set if markForDeletion() is called.  This is to help debug as well as deciding if the qtItem's UI should be updated or not.

### qtInputItem Changes
* If the space reserved for the label width is less than 1/2 the space required. The size hint is ignored and enough space for the entire label is used.
* Added Item View Option ExpressionOnly to indicate that the item must be assigned to an expression and not to a constant value
```xml
<?xml version="1.0"?>
<!--Created by XmlV4StringWriter-->
<SMTK_AttributeResource Version="4">
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="doubleFunc" Association="None"/>
    <AttDef Type="A" Label="A" BaseType="" Unique="false">
      <ItemDefinitions>
         <Double Name="d1" Label="Expression Double">
          <ExpressionType>doubleFunc</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>
   </Definitions>
  <!--**********  Attribute Instances ***********-->
  <Views>
    <View Type="Instanced" Title="DoubleItemTest" Label="Simple Double Item Test" TopLevel="true">
      <InstancedAttributes>
        <Att Type="A" Name="doubleTestAttribute">
          <ItemViews>
            <View Item="d1" ExpressionOnly="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
```

### qtAttributeItemInfo Changes
* Now stores a QPointer  to a qtBaseAttributeView instead of a qtBaseView - this is more conceptually consistent and eliminated the need for dynamic casts.
* Changed the API not to require it to be passed as a QPointer which was also unnecessary.
* baseView method no longer returns a QPointer to a qtBaseAttributeView.  It now returns a raw pointer.  There was no benefit using QPointer and it made it difficult for the compiler to properly down cast.

### qtGroupItem Changes
* The first Column is no longer marked with 1 for extensible groups.
* Fixed issue with updating extensible qtGroupItems due to the number of columns being set to 0 instead of 1
* Added the ability to load values from a  file using the ItemView option: ImportFromFile="true"  When set you can use following additional options:
    * LoadButtonText - for setting the name of the load button.  The default is "Load from File"
    * FileFormat - defines the format of the file to be read.  The options are:
        * csv - string, integer, double data separated by a separation character (Default)
        * double - a more flexible file format that contains only doubles.  They are separated by either white space or by an optional separator character
        * Note - that the above is case insensitive so you can use for example CSV, csv, or Csv
    * BrowserTitle - for setting the title of the file browser window.  The default is "Load from File..."
    * ValueSeparator - for defining the separation character.  The default is ",".
    * CommentChar - for defining the comment character that indicates that a line is a comment.  Line that start with this character are quietly skipped (not reported to the user). The default is '#'.
    * FileExtensions - for defining the list of file extensions to be allowed in the file browser.  The default is "Data Files (*.csv *.dat *.txt);;All files (*.*)"

```xml
        <Att Name="outputs-att" Type="outputs">
          <ItemViews>
            <View Path="/output-times" ImportFromFile="true" LoadButtonText="Import from File"
              FileFormat="Double" BrowserTitle="Import from Double File"
              ValueSeparator="," CommentChar="#" FileExtensions="Time Files (*.txt *.csv *.dat)"/>
          </ItemViews>
        </Att>
```

#### Added the ability to limit the min size of an extensible group's table
Added MinNumberOfRows="n" to restrict the size.  Note that if n = -1 (the default) the size is set to the total number of rows.

```xml
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="fn.initial-condition.temperature" BaseType="" Label="Temperature">
      <ItemDefinitions>
        <Group Name="tabular-data" Label="Tabular Data" Extensible="true" NumberOfRequiredGroups="2">
          <ItemDefinitions>
            <Double Name="X" Label="Indep. Variable" NumberOfRequiredValues="1"></Double>
            <Double Name="Value" Label="Temperature" NumberOfRequiredValues="1"></Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Instanced" Title="Test" TopLevel="true">
      <InstancedAttributes>
        <Att Name="temp" Type="fn.initial-condition.temperature">
          <ItemViews>
            <View Path="/tabular-data" MinNumberOfRows="9" />
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
```
#### Replaced the use of QGroupBox
The QGroupBox had several issues that was impacting the UI (see [here](https://discourse.kitware.com/t/changing-qtgroupitem/623) for more information.  And has now been replaced by a collection for QFrames.  In addition, an alert Icon has been added to indicate if the underlying GroupItem has failed to meet it's conditional requirement.
### FileItemChanges
Added ItemView Options to control aspects of the GUI:

1. ShowRecentFiles="true/false" - if the FileItemDefinition **ShouldExist** option is enabled,
   this option, when true, will display previous values using a combobox - default is false
2. ShowFileExtensions="true/false" - if the FileItemDefinition **ShouldExist** option is not
   enabled, and there are a set of suffixes associated with the item, this option, when true,
   will display the suffixes using a combobox - default is false
3. UseFileDirectory="true/false" - if true and if the FileItem value is set, the file browser will open in
   the directory of the current value.  If the current value's directory does not exist the browser
   will either use the DefaultDirectoryProperty value or revert to its default behavior - default
   is true
4. DefaultDirectoryProperty="nameOfResourceStringProperty" - if the FileItem value is set and the
   string vector property value on the item's attribute resource exists and refers to an existing directory,
   the file browser will open in the directory referred to by the property value

See fileItemExample.sbt and fileItemExample.smtk as examples.  Note that to demonstrate
DefaultDirectoryProperty using these files you will need to create a string property called
testDir and set it to something valid.

Here is a snippet of fileItemExample.sbt showing the item view section:

```xml
  <Views>
    <View Type="Attribute" Title="A" Label="A Atts"
      SearchBoxText="Search by name...">
      <AttributeTypes>
        <Att Type="A">
          <ItemViews>
            <View Item="f0" ShowRecentFiles="true"/>
            <View Item="f0a" ShowRecentFiles="true" ShowFileExtensions="true"/>
            <View Item="f0b" ShowRecentFiles="true" ShowFileExtensions="true"/>
            <View Item="f1" ShowRecentFiles="true"/>
            <View Item="f3" DefaultDirectoryProperty="testDir"/>
          </ItemViews>
        </Att>
      </AttributeTypes>
    </View>

```

### Overriding ItemViews
You can now specify ItemViews with an Item's Configuration.  If there was already an ItemView for a specific Item, it will be overridden.  Below is an example."

```xml
        <Att Name="Test Attribute" Type="test">
          <ItemViews>
            <View Item="a" Type="Default" Option="SpinBox"/>
            <View Path="/b" ReadOnly="true"/>
            <View Path="/c">
              <ItemViews>
                <View Path="/d" FixedWidth="20"/>
                <View Item="e" Option="SpinBox"/>
              </ItemViews>
            </View>
            <View Path="/f/f-a" Option="SpinBox"/>
            <View Path="/f/f-b" FixedWidth="10"/>
            <View Path="/f/f-c/f.d" FixedWidth="0"/>
            <View Path="/f/f-c/f.e" Option="SpinBox"/>
            <View Path="/f/f-c/f.f/f.f.g" ReadOnly="true"/>
            <View Path="/f/f-c/f.f/f.f.h" FixedWidth="0"/>
          </ItemViews>
        </Att>
```
Note that item /c contain ItemView for its children.

### Changed to qtAssociation Widget
* Added the ability to indicate that all persistent objects that can be associated to a particular type of attribute must be.
* Re-factored qtAssociationWdiget into an abstract class and a concrete implementation called qtAssociation2ColumnWidget - this should allow for easier customization
* Added a pure virtual method to indicate the widget should display available persistent objects based on a definition - useful if you want the widget to indicate what still needs to associated to an attribute if no attribute is selected.
* Added the ability to customize the following aspects of qtAssociation2ColumnWidget
  * Title Label
  * Current Column Label
  * Available Column Label
* Added allAssociationMode to qtAssociation2ColumnWidget to indicate that all relevant persistent objects must be associated to a type of attribute else display the warning icon

Here is an example of customizing the AssociationWidget for an AttributeView:

```xml
    <View Type="Attribute" Title="HT Boundary Conditions" Label="Boundary"
      RequireAllAssociated="true" AvailableLabel="Surfaces that still require boundary conditions"
      CurrentLabel="Surfaces associated with the current boundary condition"
      AssociationTitle="Boundary condition association information">
```
* qtAssociation2ColumnWidget will now remove all invalid values from the attribute's association information.

### qtReferenceItemComboBox has been renamed to qtReferenceItemEditor
The main reason for the change is that this class now supports the ability of ReferenceItems
having optional activeChildren

### Add ObjectGroupPhraseContent and QueryFilterSubphraseGenerator

A new PhraseContent subclass is introduced which groups a set of
persistent objects for user presentation. This ObjectGroupPhraseContent
class takes resource and component filter strings plus a title and
populates its children with objects that match the filters.

A new SubPhraseGenerator subclass is introduced which generates
subphrases by querying the filter in ObjectGroupPhraseContent.

Ex. Show a list of components who has a string property as "selectable".

Now PhraseContent and DescriptivePhrase classes learn the ability
to get the undecoratedContent directly via undecoratedContent()
function.

### Badges for descriptive phrases

This work adds support for multiple arbitary, run-time-configurable badges
to descriptive phrases. There is also supporting work that refactors and
simplifies how PhraseModel subclasses are constructed and configured.

+ Add Badge, BadgeSet, and BadgeFactory classes.
+ Refactor PhraseModel registration in view::Manager into PhraseModelFactory
  using the new smtk::common::Factory template.
+ Refactor PhraseModel construction so that
    + constructors, not static `create` methods, are used; and
    + constructors take and use a view::Configuration object that they
      use to prepare themselves and objects they own (namely, BadgeSet
      and (indirectly) SubphraseGenerator).
+ Add a badge (ObjectIconBadge) showing icons based on the phrase's subject.
  This uses the view-manager's icon factory.
+ Add a badge (AssociationBadge) showing an exclamation mark when
  matching persistent objects are not associated to attributes with a
  given set of definitions.
+ Replace VisibilityContent decorator with VisibilityBadge.
+ Enhance VisibilityBadge so it also applies to all smtk::geometry::Resources
  instead of just mesh and model resources. We special-case attribute
  resources so Attributes do not have visibility eyeballs unless they explicitly
  have geometry.
+ Use TypeAndColorBadge to replace the hard-coded icon for displaying type
  and setting color.
+ The badge action method API has changed to include a BadgeAction
  reference with each call and requires a boolean return value.
  You will need to update your Badge's action() method to return
  true only when you are able to cast the BadgeAction passed to
  your badge to a supported type. You should always support
  BadgeActionToggle.
+ This API change was made so that other actions and bulk actions
  could be supported. In particular, the qtResourceBrowser now
  invokes BadgeActionToggle on both mouse click and keypress (space
  bar) events, providing bulk phrase actuation.
+ In general, Since keypresses may occur without the mouse over any
  badge, some user interface components may pass an action to all
  badges in a BadgeSet (in order) until one responds by returning true.

### PhraseModel::handle*() without needing a ComponentItem

smtk::view::PhraseModel and its subclasses stay up to date with Resources in
SMTK by observing completed Resource events and Operations. Previously, the
methods, handleCreated(), handleModified(), and handleExpunged() required a
ComponentItem in order to extract Components which had been created, modified,
and expunged, respectively. The need for a ComponentItem has been removed by
allowing these methods to accept a smtk::resource::PersistentObjectSet. The
intention is to enable subclasses of PhraseModel to observe events of their
own choosing which may not yield a ComponentItem as Operation::Result does.

#### Developer changes

Developers must now extract the created, modified, and expunged Components in
overrides of PhraseModel::handleOperationEvent() when observing Operations.


### ReferenceItemPhraseModel
Added a phrase model that uses the associatable objects method in the new smtk::attribute::Utilities class. This method takes uniqueness into consideration.

### Naming Qt Widgets
Many Qt widgets in SMTK's GUI extensions are now assigned object names
  so that testing can be more reliable. **Note that if you have existing
  XML tests, these may need to be updated to reflect the new names**.
  The new names include attribute names and item names, which also makes
  the test XML easier to understand.

### Operation dialog

A qtOperationDialog class is addded for launching SMTK operations from a
modal dialog. The intended use is for modelbuilder plugins that use menu or
similar actions to invoke SMTK operations. (For example, export operations
are typically run from a modal dialog.) The qtOperationDialog is created
as a QTabWidget containing 2 tabs. The first tab embeds a qtOperationView
for the SMTK operation, and the second tab displays the operation's "info"
as formatted by qtViewInfoDialog.

### Done button for operation editor

The qtOperationView widget now includes a "Done" button.
Clicking it emits a signal that the parent pqSMTKOperationPanel
uses to delete the qtOperationView and its widgets.

### Other Changes
+ An error in the logic of `PhraseModel::updateChildren()` was fixed that
  could sometimes result in infinite loops trying to report reorders of an
  item's subphrases.


## Graph-Model Resource

This release of SMTK includes a flexible new approach to modeling;
rather than a fixed set of model components which assume a CAD-style
boundary representation, the `smtk::graph::Resource` allows an
extensible set of components (nodes) connected by relationships (arcs)
that are constrained by the types of nodes at their endpoints.

See the user's guide, the unit tests, and the OpenCASCADE session for
more information.

## New Geometry System
### Introduce the Geometry System

SMTK now has a geometry subsystem.

We are working toward removing smtk::model::Tessellation in order
to resolve performance problems inherent in its design.
In its place is smtk::geometry::Geometry and subclasses that allow
developers an efficient (usually zero-copy) technique for providing
access to their geometry directly in the format that rendering
and analysis backends will use.

This first step simply adds smtk::geometry::Geometry but does
not force its use or remove smtk::model::Tessellation.
See `doc/userguide/geometry` for more information.

Additionally, resources may now associate any kind of VTK data
object with themselves or their components. Preliminary support
for rendering image data as 3 axis-aligned slices is now present
as a prototype (slice planes cannot yet be edited and only the
first image in a resource is rendered). In the future, this
support will be expanded.

### Geometry subsystem queries

The new geometry system in SMTK takes advantage of the
new query classes exposed in `smtk::resource::Resource::queries()`.
APIs are exposed for bounding boxes, closest point on discretization,
distance to (continuous) geometry, and "selection footprint."

The latter (selection footprint) is a query now used by the SMTK
representation to replace fixed-functionality methods that determined
what to highlight when a resource or component with no renderable
geometry was selected. This allows plugins that expose new resource
types to add their own logic.

### Other Changes
+ Added a convenience method to `geometry::Resource` to
  fetch the first available `geometry::Geometry` object
  for use in querying geometric bounds.
+ Fixed a situation where adding a backend to `geometry::Manager`
  after resources have been added to the linked `resource::Manager`
  would not construct `geometry::Geometry` for the new backend.

## Software Process Changes
### Advanced cmake option SMTK\_ENABLE\_OPERATION\_THREADS

We have added an advanced cmake option to enable/disable running SMTK
operations asynchronously from a thread pool. Previously, operations
were *always* run asynchronously when the `smtk::extension::qtViewRegistrar`
feature was initialized. With the new option, applications can explcitly
disable asynchronous operations by setting `SMTK_ENABLE_OPERATION_THREADS`
to OFF. The option is available when Qt is enabled (`SMTK_ENABLE_QT_SUPPORT`)
and is ON by default.

### Testing Infrastructure

A new CMake macro named `smtk_build_failure_tests` is provided in `CMake/SMTKTestingMacros.cmake`.
See `doc/userguide/contributing/testing.rst` for details.

## ParaView Related Changes
### VisibilityBadge fixed to handle multiple resources

It is not an error for a SelectionFootprint query to return
components from other resources (e.g., an assembly resource
might have components that do not contain geometry but instead
reference the geometry of components assembled from other
resources). In this case, we should attempt to set the visibility
of those components in their matching resource's representation.
The VisibilityBadge now does this instead of attempting to set
the visibility of a component in its own representation (which
has no effect since the component does not exist in that scope).

### Other Changes
+ Changes to pqDoubleLineEdit required changes to SMTK's cone and handle widgets.
+ A change in ParaView's pqPointPickingHelper has removed the
  setShortcutEnabled method. If you were previously connecting
  this method to a pqInteractivePropertyWidget subclass (like
  the cone widget in SMTK), replace that signal connection
  with an instantiation of pqPointPickingVisibilityHelper.
  See `smtk/extension/paraview/widgets/plugin/pqConePropertyWidget.cxx`
  for an example of its usage.


## Changes to SMTK Plugins
### Changes to smtkPVServerExtPlugin
 Some functionality in smtkPVServerExtPlugin has been split into
  new plugins named smtkPVModelExtPlugin and smtkPVMeshExtPlugin.
  If your application previously packaged smtkPVServerExtPlugin, you
  will want to consider packaging these new plugins. If you do, then
  previous functionality will be preserved. If not, then some operations
  from the mesh and model subsystem will no longer be present (including
  selection-responder operations) — assuming that other plugins did not
  introduce dependencies on smtk::model::Registrar or smtk::mesh::Registrar,
  respectively.

  As an example, omitting the new smtkPVMeshExtPlugin (in addition to
  the polygon-session, mesh-session, oscillator-session, and delaunay
  plugins) results in the mesh import operation being absent; users will
  not be prompted to choose a reader for file formats like STL, PLY, or
  OBJ files if the application provides other import operations that handle
  them. The reader-selection dialog can confuse users, so preventing multiple
  operations in the ImporterGroup from handling a file type is desirable.

## Representation Selection Style Customization

Add customization to `vtkSMTKResourceRepresentation` to allow plugins to over-ride how the 3D selection is displayed.
Plugins may register a `vtkSMTKRepresentationStyleSupplier` to change the selection geometry.
The RGG session plugin is using this functionality to not show the selection at all in the 3D view.


### Other Changes
* SMTK's plugin system now facilitates the registration of new managers in addition to registering
things to existing managers.

## Changes to Document Generation
### API Doxygen docs

A global pass to remove warnings generated by Doxygen during the
api-document generation process. Warnings about undocumented content
have been turned off, so developers should be able to generate documentation
and see what warnings their new code produces.

* Links to all common shared pointer typedefs refer to their contained type.
* Use a python package of git-lfs so readthedocs.io can see our pngs.
