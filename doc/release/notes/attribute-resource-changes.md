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

#### Support for ReferenceItems within detached Attributes
When an attribute containing ReferenceItems (including associations)
is detached, the links describing the connections between the
ReferenceItems and their references are now severed (originally, this
was only true for ReferenceItems representing associations). The ReferenceItems'
caches remain populated after detachment to support the
ReferenceItems' API once its parent attribute is removed from its
Resource.
