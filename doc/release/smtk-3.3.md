# SMTK 3.3 Release Notes
SMTK 3.3 is a minor release with new features.  Note any non-backward compatible changes are in bold italics. See also [SMTK 3.2 Release Notes](smtk-3.2.md).

## Changes to SMTK's Resource System
### Introduction of Properties for Resources and Components

Resources and components now have access to Properties, a
dictionary-like container that can hold any copy-constructible
type. Currently enabled types include long, double, std::string, and
std::vectors of these three types. Resource and component properties
replace and extend the functionality of smtk::model's property system,
while maintaining much of smtk::model's property API. For more
information, see the Properties section of the Resources description
in the user guide.

## Changes to Operations
### Removal of MarkModified operation
The MarkModified operation has been removed since it was considered redundant.  The attribute Signal operation should be used instead.

### Removal of "Operation Created" Event

Operation creation is often performed within observation events (so
the created operation can be added to the launcher queue), so it is
possible to deadlock ModelBuilder by repeatedly create operations when
an operation is running. Since no call sites specifically filter for
this event type, it has been removed.

## Changes to SMTK's Observer System
### Resource & Operation Observer signature

Instead of shared pointers, **Resource and Operation Observers have been
updated to pass const references to Resources and Operations,
respectively.** This change in signature has been made to help inform
downstream developers about the appropriate access priveleges
associated with Resource and Operation observation (Observers should
observe, but not own, the things they observe).

#### Developer changes

Because a downstream developer can acquire a shared pointer to a
Resource or Operation by calling `shared_from_this()`, the change is
largely syntactic. The change is made to guide the developer about
what he *should* do with these objects, not what he *can* do with
them.

### Descriptions for Observers

As one of its primary entrypoints, SMTK's Observer pattern enables
consuming projects to dynamically interact with SMTK objects. Because
Observers have the ability to change the functionality of an
application, they are a common site for debugging. We have added API
to Observers to describe an Observer as it is inserted, allowing for
the query of an observer's function during execution.

#### Developer changes

Observers now accept an optional additional string for briefly
describing their function. Additionally, the ADD_OBSERVER macro has
been introduced to inject the insertion location as an Observer's
description.

## Attribute Resource Changes
### Removal of Redundant Item/ItemDefinition Classes
#### RefItem and RefItemDefinition
With the addition of ComponentItem (which can refer to a component of a resource) there is no longer a need for RefItem (which can only refer to an attribute). To update existing code to remove RefItems and their Definition you need to simply do the following:
Assume you wanted to reference attributes that are derived from a definition called base. In the original code you might have:

```
auto oldRefDefinition =
attDef->addItemDefinition<smtk::attribute::RefItemDefinitionPtr>("BaseDefItem");
oldRefDefinition->setAttributeDefinition(base);
```
In the updated code you would have:

```
auto compDef =
attDef->addItemDefinition<smtk::attribute::ComponentItemDefinitionPtr>("BaseDefItem");
std::string attQuery = resource.createAttributeQuery(base);
compDef->setAcceptsEntries(smtk::common::typeName<smtk::attribute::Resource>(), attQuery, true);
```
Note that SMTK's XML and JSON I/O classes will convert RefItem and RefItem definitions into their equivalent ComponentItem and ComponentItemDefinition forms.

##### Change in Copying Behavior
When copying an AttRef item or definition, there was an option to copy the item or definition being referenced.  This was required since AttRefItemDefinition held a pointer to the referenced definition and likewise an AttRefItem held a pointer to the item being referenced.  Since ComponentItemDefinitions do not have this requirement, copying a ComponentItemDefinition ***does not*** copy the corresponding definition.  Likewise  a copied ComponentItem will refer to the same component has the original item instead of providing an option to make a copy of the referenced component and have the copied item refer to it.

#### MeshSelectionItem and MeshSelectionItemDefinition
This item was a stop gap for returning parts of a model's tessellation which is now handled using the selection process. There is no conversion for the item and its definition.

#### MeshItem and MeshItemDefinition
As with AttRefItem and AttRefItemDefiniiton, there is no longer a need for MeshItem (which can only refer to a mesh entity). To update existing code to remove MeshItems and their Definition you need to simply do the following:


```
auto oldMeshDefinition =
attDef->addItemDefinition<smtk::attribute::MeshItemDefinitionPtr>("MeshItem");
```
In the updated code you would have:

```
auto compDef =
meshDef->addItemDefinition<smtk::attribute::ComponentItemDefinitionPtr>("MeshItem");
compDef->setAcceptsEntries(smtk::common::typeName<smtk::mesh::Resource>(), "meshset", true);
```

### Category Changes to Attribute Resource

* Categories can now be assigned to Attribute Definitions and Group Item Definitions
  * A Category assigned to an Attribute Definition, Value Item Definition, or Group Item Definition is also (by default) inherited by their child item definitions.  This will simplify  creating template files since the author will now be able to state that attributes (and their items) are of category "X" with a single command/specification instead of having to explicitly  assign "X" to all of the item definitions associated with the attribute definition.
  * Template authors will no longer have to be careful to specify categories to optional children items of a value item since they can now inherit those assigned to the value item itself.
* Categories assigned explicitly to an attribute definition or item definition are now referred to as **Local Categories**. The set of categories (both explicit and inherited) are referred to as **Categories**.
  * **smtk::attribute::ItemDefinition::addCategory(...) has been replaced**. The new method is mtk::attribute::ItemDefinition::addLocalCategory(...)
  * **smtk::attribute::ItemDefinition::removeCategory(...) has been replaced**. The new method is mtk::attribute::ItemDefinition::removeLocalCategory(...)
  * A new method **smtk::attribute::ItemDefinition::localCategories()** has been added.  This returns all categories explicitly assigned to the item definition.  * You can control whether an item definition should inherit the categories from its owning attribute definition and Item Definition (in the case of Group or Value Item children)
    *  **smtk::attribute::ItemDefinition::isOkToInherit()** returns true if it's ok to inherit categories from its parent.  **Default is true**
    *  **smtk::attribute::ItemDefinition::setIsOkToInherit(bool)** for setting the item definition's category inheritance behavior
  * Added methods to attribute::Definition for specifying its local categories
    * **smtk::attribute::Definition::addLocalCategory(...)** - adds a local category to the Definition
    * **smtk::attribute::Definition::removeLocalCategory(...)** - removes a local category from the Definition
    * **smtk::attribute::Definition::localCategories()** - returns the local categories assigned to the Definition

* **Definition and Item Definition updateCategories() method have been replaced**. The new methods are:
  * attribute::Definition::applyCategories(...) and attribute::ItemDefinition::applyCategories(...)

#### Inheritance Rules
* A Definition will inherit all of the local categories associated with its Item Definitions (and their descendants) along with all of the categories associated with it's base definition
* An Item Definition will inherit all of local categories associated with it's children item definitions (and their descendants) and if it's isOkToInherit mode is true it will also inherit all local categories assigned to it's parent (and their ancestor up to and including the first ancestor whose isOkToInherit mode is false).  Note that this could include local categories associated with it's owning attribute Definition and it's base definition

#### Example

Consider the following scenario where (...) denotes a local category

* Definition A (A)
  * Group Item Definition g1 (g1)
     * String Item Definition s1 (s1)
         * String Item Definition s2 (s2) - isOkToInherit (false)
             * Void Item Definition v1 (v1)
     * String Item Definition s3 (s3) - isOkToInherit (false)
* Definition B (B) - base definition is A
  * Void Item Definition v2 (v2)

The resulting categories would be:

* A: A, g1, s1, s2, v1, s3
* B: A, g1, s1, s2, v1, s3, B, v2
* g1: A, g1, s1, s2, v1, s3 (same as A)
* s1: A, g1, s1, , s2, v1
* s2: s2, v1
* s3: s3
* v1: s2, v1
* v2: A, B, v2

### Extending Advance Level Support

A full description on how the new advance level support works can be found [here] (https://discourse.kitware.com/t/supporting-advance-read-and-write-at-the-attribute-level/346)

For examples, see attribute/testing/cxx/unitAdvanceLevelTest.cxx and data/attribute/attribute_collection/unitAttributeAdvanceLevelTest.sbi

* Advance Level Information can now be inherited by a Definition from its base Definition
* Advance Level Information can now be inherited by an Item Definition from its Attribute Definition
* Advance Level Information can now be inherited by an Item Definition from its owning Item Definition such as a Group Item Definition and/or Value Item Definition
* Advance Level of an Item is now also based on the advance level of its owning Item or owning Attribute
* **Advance Levels are know represented as unsigned integers instead of signed integers**

#### Local Advance Level Information
Attribute Definition, Item Definition, Attribute and Item now have the ability to have local advance level information for both GUI read and write access.  Related methods include:

* setLocalAdvanceLevel: **note that this replaces setAdvanceLevel methods**
* unsetLocalAdvanceLevel
* hasAdvanceLevelInfo

Local advance level information "overrides" the information that would be inherited. For example setting local advance read level for a Definition will override the advance read level that would have been inherited from its base Definition.  Similarly setting the local advance write level for an Attribute will override the advance write level that would have been inherited from its Definition.

#### Item's advanceLevel
An Item's Advance Level is now the max of its local level (or if not set, it's definition) and the advance level of its owning Item or owning Attribute.

#### Applying Advance Levels
Definition and ItemDefinition has methods for pushing its advance level information to their children Definitions and Item Definitions.  These method are used by Resource's finalizeDefinitions method.

#### XML and JSON Support
The same format used for setting local Advance Levels for Items is used for Attribute, Definition, and Item Definition

``` xml
    <AttDef Type="A" Label="A" BaseType="" Version="0" AdvanceWriteLevel="1" Unique="false"\>

```

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

#### Unset Value Error for Reference Item Iterator
A custom exception is now thrown when an attempt is made to
dereference an iterator to an unset reference item. This exception can
be caught by consuming code (for an example, see the
unitUnsetValueError test). An isSet() method is now available on
ReferenceItem's const_iterator to check the iterator's validity prior
to dereferencing.

### Other Changes
* Added a static method attribute::Resource::createAttributeQuery that will return an appropriate string for querying attributes based on a specific definition.
* FileSystemItem::ValueAsString() now returns "" when the item is not set.
* When Attribute::Attribute(...) no longer creates the attribute's items.  This is now done using the new Attribute::build() method - this allows Items to access the attribute's shared pointer when they are constructed.
* ReferenceItem now unsets it's values when being deleted so the corresponding links are removed from the resource.
  * In order for ReferenceItem to unset its values, it now holds onto a weak pointer to the attribute rather than using the attribute() method.  The reason is that Items that are owned by other Items lose their connection to the attribute when being deleted.  This ensures that the ReferenceItem will be able to access the attribute.
* Functionality for calculating the set of categories represented by an Analysis Attribute has been moved from qtAnalysisView to the Analyses class.
* Added GroupItem::prepend method that can add subGroups at the beginning instead of appending at the end of the subGroup vector.

### Bug Fixes
* Attributes where not properly release it's association information when being deleted or when updating it's association information during Definition::buildAttribute

## Model Resource Changes

### Add new features for instance glyphing support

Added custom orientation, mask, scale and per point color
support for glpyhing smtk entities.

### Instance placement

When snapping to entities with mesh tessellations, support has been
added to snap to the nearest point on the entity's surface (rather
than the nearest point explicitly defined in the tessellation).

### Instance editing

In addition to creating instances, it is now possible to
+ divide an instance by creating a point or cell selection in a
  render window and running the "divide instance" operation;
+ change an instance's prototype;
+ merge multiple instances with the same prototype into a
  single tabular instance.

### Fix the glyphing visibility && selection and add hidden notion to EntityRef

* The visibility control of glyphs has been fixed.
* The rendering of selected/hovered glyphs has been fixed.
* A new entry as hidden property is added to EntityRef. Developers can choose if the vtk representation for this EntityRef should be hidden from geometry generation stage or not(It can still be used for glyphing. See latter explanations). A use case is that an entity should be just used for glyphing only as its own vtk geometry representation makes no sense standalone. So it will not be added to the model multiblock source but its representation can be generated and added to the prototype multiblock source when used as a glyphing prototype. At the same time, developers can choose if the visibility control for this EntityRef should be hidden from the view control in downstream applications(Ex. CMB's model panel).
The APIs which allow developers to hide the entity from tessellation generation /andd view presentation are as below:

// Assume we have a valid EntityRef as ent
EntityRef ent = GetFromSomePlace();

// Hide ent from tessellation generation
ent.setHiddenOptionsStatus(true, HiddenOptions::HideTessellationGeneration);
// Hide ent from view presentation
ent.setHiddenOptionsStatus(true, HiddenOptions::HideViewRepresentation);

// Hide ent from all stages(For now it includes tessellation generation and view presentation)
ent.setHiddenOptionsStatus(true);

// Query all its hidden options status
int hiddenOptions = ent.hiddenOptionsStatus();

// Query a specific hidden option status. 1 means isSet and 0 means notSet.
int hideFromTessGenStatus = ent.hiddenOptionsStatus(HiddenOptions::HideTessellationGeneration);

### Extensions to point Locator base class

The point locator base class has been extended for implementations to
provide routines for constructing uniform random point samples on a
model surface. An implementation for this method currently exists for
models with a mesh-based tessellation.

## Mesh Resource Changes

### Cell Selection

Mesh cells can now be selected and used as input for operations. When
cells are selected, a meshset is constructed to contain the
selection. Once the selection is no longer referenced by any
operations, it is automatically deleted.

### Extract by Dihedral Angle

Given a 2-diensional triangular meshset, this operation traverses the
meshset and accumulates all neighboring cells whose dihedral angle is
less than a user-defined value.

### Extract Adjacency

Given a meshset and a desired dimensionality <d>, this operation computes
and returns a meshset containing the <d>-dimensional cells adjacent to
the input mesh.

### Extract Skin

Given a <d>-dimensional meshset, this operation computes a meshset
comprised of the <d-1>-dimensional exterior adjacency cells of the
input mesh.

### Point Locator implementation for models with mesh tessellations

An implementation of the abstract point locator class has been
implemented using Moab's point locator. This allows models that have
associated mesh tessellations to snap points to model surfaces without
having to load SMTK's ParaView extensions.

## Mesh Session Changes

### Merge Operation

There is now an operation that merges model entities of like dimension.

### Subtract Operation

There is now an operation that subtracts meshsets from other meshsets.


## I/O Changes

### smtk::io::Logger is now thread-safe
The class now does a mutex lock when modifying or accessing its records or its underlying stream.  Care must be taking when redirecting the logger's stream to avoid deadlocks.  For example using smtk::extension::qtEmittingStringBuffer, you should make sure to use Qt::QueuedConnection when doing a QObject::connect to the buffer's flush signal.  See smtk/extension/qt/cxx/testing/UnitTestEmittingStringBuffer.{h,cxx} for an example.
#### Changes to Logger API
* **Logger::records() has been changed to return a copy of the Logger's records instead of a const reference to them which is not thread safe.**
* **Logger::record(int i) now returns a copy of the ith record instead of a const reference to it.**

### New Attribute Resource and SBT formats (4.0)
We have now create version 4.0 for both attribute SBT and attribute SMTK files.  This will now be the default in terms of writing out attribute information.  SMTK will still support reading in Versions 1, 2, and 3.

### Introduction of Archives

SMTK now has an Archive class to represent a portable collection of
files that are stored as a single file on-disk. An archive is
described by its filesystem path. Once instantiated, a user can insert
files into the archive, serialize/deserialize the archive to/from
disk, access a list of files in the archive, and acquire file streams
to these files by accessing them via their name. An archive can be
considered a directory containing files; as such, each file in the
archive must be assigned a unique path.

#### Defining and using Unique Roles for ComponentItems
##### To add unique roles 10 and 20 for an Attribute Resource:
```
<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">

  <!-- Category & Analysis specifications -->
  <Categories>
    <Cat>Enclosure Radiation</Cat>
    <Cat>Fluid Flow</Cat>
    <Cat>Heat Transfer</Cat>
    <Cat>Induction Heating</Cat>
    <Cat>Solid Mechanics</Cat>
  </Categories>

  <Analyses>
    <Analysis Type="Heat Transfer">
      <Cat>Heat Transfer</Cat>
    </Analysis>
    <Analysis Type="Enclosure Radiation" BaseType="Heat Transfer">
      <Cat>Enclosure Radiation</Cat>
    </Analysis>
    <Analysis Type="Induction Heating" BaseType="Heat Transfer">
      <Cat>Induction Heating</Cat>
    </Analysis>
    <Analysis Type="Fluid Flow">
      <Cat>Fluid Flow</Cat>
    </Analysis>
    <Analysis Type="Solid Mechanics">
      <Cat>Solid Mechanics</Cat>
    </Analysis>
  </Analyses>

  <UniqueRoles>
    <Role ID="10"/>
    <Role ID="20"/>
  </UniqueRoles>
```
##### Defining the role for a ComponentItemDefinition
```
        <Component Name="uniqueTest" Label="Unique Test" Role="10">
          <Categories>
            <Cat>Heat Transfer</Cat>
          </Categories>
          <Accepts>
            <Resource Name="smtk::model::Resource" Filter="face"/>
          </Accepts>
        </Component>
```
### Specifying Category Checking Options for ItemDefinitions
In the example below, Items created from Item Definition s1 will pass their category checks if either category a or b is included in the input set of categories.  In the case of Item Definition s2, the corresponding Items will pass their checks if the input set contains both a and b.

```
<ItemDefinitions>
  <String Name="s1" CategoryCheckMode="Any">
    <Categories>
      <Cat>b</Cat>
      <Cat>c</Cat>
    </Categories>
  </String>
  <String Name="s2" CategoryCheckMode="All">
    <Categories>
      <Cat>a</Cat>
      <Cat>b</Cat>
    </Categories>
  </String>
</ItemDefinitions>

```

#### Changes
* ValueItem's expressions are now saved using ComponentItem format to reflect the fact that AttRefItems are no longer supported.
* MeshItems and MeshSelections are no longer supported in XML and JSON

## Changes to the View System and Qt Extensions

### Changes to qtBaseView (and Introducing qtBaseAttributeView class)

* The qtBaseView class has been split into qtBaseView and qtBaseAttributeView.
  All of the existing qtBaseView subclasses now instead inherit qtBaseAttributeView.
* The displayItem test now calls 2 new methods categoryTest and advanceLevelTest.  This makes it easier for derived classes to override the filtering behavior

### Changes to qtBaseAttributeView
#### IgnoreCategory Mechanism
Added an ignoreCategories mechanism so that designers have the option to have Views not filter base on categories as shown below:

```xml
   <View Type="Attribute" Title="Configurations" IgnoreCategories="true">
      <AttributeTypes>
        <Att Type="Analysis" />
      </AttributeTypes>
    </View>
```
#### Added the concept of Configurations for Top Level Views
Similar to the Analysis View, Configurations provide a mechanism to define a set of categories to filter information defined in the Views.  To use Configurations, specify **UseConfigurations** in the top level view.  **ConfigurationType** is used to define the Attribute Definition Type name for configuration attributes.  Unlike an Analysis View which represents a single configuration, the configuration mechanism supports multiple configurations.

Configurations are displayed as a combobox.  Configurations can be created during run time using the **CreateConfigurations** View attribute.  **ConfigurationLabel** can be used to define the label displayed next to the configuration combobox.  The selected configuration is represented as a long Property named **_selectedConfiguration** assigned to the attribute with a value of 1. Below is an example top-level view using Configurations:

```xml
    <View Type="Group" Title="TopLevel" TopLevel="true" TabPosition="North"
      FilterByAdvanceLevel="true" UseConfigurations="true" ConfigurationType="Analysis"
      ConfigurationLabel="My Configurations:" CreateConfigurations="true">
      <Views>
        <View Title="Test" />
        <View Title="Configurations" />
      </Views>
    </View>
```

You can use an Attribute View to edit existing Configurations by setting the attribute view type to the same as the ConfigurationType:

```xml
   <View Type="Attribute" Title="Configurations" IgnoreCategories="true">
      <AttributeTypes>
        <Att Type="Analysis" />
      </AttributeTypes>
    </View>
```
See **data/attribute/attribute_collection/ConfigurationTest.sbt** as an example template file.

##### Current Limitations
* When using an Attribute View to define and edit Configuration Attributes, if the user only creates an attribute using the Attribute View and does not edit any of its items, it will not be automatically added to the configuration combobox.

#### Added Item names to attributeChanged method
The names of the items being modified are now returned by the Signal operator. This is now used by the AttributeView class.

#### Supporting extensions to Advance Levels
* qtItem widgets now check to see if they are writable based on their Item's advance write level and the current advance level.  If they are not or if their ItemView has the readonly property, they will be readonly.
* If the current advance level is below an attribute's advance write level, then the attribute will not be eligible for deletion in an AttributeView.

### Changes to displaying double items
Using ItemViews you can now control how the double value item is displayed based using the following "attributes":

* Notation - general display behavior.  Supported values include:
 * Fixed - displays the value in fixed notation.  This is equivalent to printf's %f flag
 * Scientific - displays the value in scientific notation.  This is equivalent to printf's %e flag
 * Mixed - tries to determine the best notation to use.  This is equivalent to printf's %g flag
* Precision - controls the precision (in the case of Fixed and Scientific Notations) or significant digits (in the case of Mixed Notation) that are to be displayed when the value is not being edited.
* EditPrecision - controls the precision (in the case of Fixed and Scientific Notations) or significant digits (in the case of Mixed Notation) that are to be displayed when the value is being edited.

Example SBT Code:

```xml
    <View Type="Instanced" Title="General">
      <InstancedAttributes>
        <Att Name="numerics-att" Type="numerics">
          <ItemViews>
            <View Item="dt_init" Type="Default" Precision="6" EditPrecision="10"/>
            <View Item="dt_max" Type="Default" Precision="6" EditPrecision="10" Notation="Fixed"/>
            <View Item="dt_min" Type="Default" Precision="6" EditPrecision="10" Notation="Scientific"/>
          </ItemViews>
        </Att>
        <Att Name="outputs-att" Type="outputs" />
        <Att Name="simulation-control-att" Type="simulation-control" />
<!--         <Att Name="Mesh" Type="mesh" /> -->
      </InstancedAttributes>
    </View>
```
See [SMTK Issue 270 to see what the resulting UI looks like.](https://gitlab.kitware.com/cmb/smtk/issues/270)

### Added qtAttributeEditorDialog class

This class can be used to edit a single attribute and is used to create new expressions for ValueItems (using the qtInputsItem class.  Note that the current implementation does not undo changes made to the attribute using the dialog but does tell the caller the user requested a cancelation.  In the current use case this means to delete the newly created expression attribute.

### Creating Expressions for ValueItems
With the introduction of qtAttributeEditorDialogs, it is now possible to create new expression attributes without having to change Views.

### Add GUI indicators that an operation is currently running

When an operation is in progress for a qt-enabled application, the
cursor will now change to "busy". Additionally, the icons for all write-locked
resources will temporarily change to a lock symbol.

### Add icons for applications in dark mode

For users whose system settings use dark mode, a complimentary set of
white icons has been added.

#### Developer changes

Icons should be added to SMTK in duplicate with "_b" and "_w" prefixes
to denote their use in light and dark mode, respectively.

#### User-facing changes

Users with dark mode enabled will now be able to see their icons.


### 3D point widget

The pqSMTKPointItemWidget now uses a custom subbwidget
(pqPointPropertyWidget) rather than relying on ParaView's
pqHandlePropertyWidget.  This widget is now much more compact
and includes the help string as a tooltip rather than a label.

Also, the point visibility checkbox is now a tri-state checkbox:
 + checked: the 3-d widget is visible and has keyboard shortcuts registered
 + partially checked: the 3-d widget is visible but no shortcuts are registered
 + unchecked: the 3-d widget is not visible
The point-visibility checkbox is optional — it will not be displayed unless
the item's View configuration includes `ShowControls="true"` —
and optionally ties its setting to a discrete string item
so you can save the visibility and interaction state as
part of the attribute system along with point coordinates.

#### Limitations

### Changes to Processing DiscreteItems
#### Added "Please Select" option
In addition to showing all of the possible enum values, the qtDiscreteValueEditor will include a "Please Select" option.  This is used to show that the item is not set and can be used to unset the item.

#### Processing Category and Advance Level
Enums can now be filtered out based on the category and advance level information explicitly assigned to the enum.  If the item's current value is not considered "valid" based on the current category/advance level settings, it is added to the list but is colored red to indicate that it is not considered "valid".

### Changes to qtResourceBrowser

* Now inherits qtBaseView instead of QWidget, to allow configuration via a ViewInfo.
* A default .json config specifies the default PhraseModel and SubphraseGenerator types to use.
* The smtk::view::Manager class can dynamically construct PhraseModels and SubphraseGenerators based on typename.

### Changes to qtAttributeView
#### Added ability to hide the top row Create/Copy/Delete Buttons
By using the **DisableTopButtons** the top buttons along with the attribute type selector combobox/label are not displayed.
Here is an example:
```xml
    <View Type="Attribute" Title="Advance Level Test" TopLevel="true" DisableTopButtons="true">
      <AttributeTypes>
        <Att Type="A"/>
      </AttributeTypes>
    </View>
```

This change does not provide an easy way to distinguish multiple
point-widgets in the 3-d scene yet, nor a way to force only 1 widget
at a time to register shortcuts for "P" and "Ctrl+P".
Thus, it is still possible for users to become frustrated when these
keys do not work simply because multiple widgets are visible.

### Improving Observer Stability
It was discovered that passing the **this** pointer into an Observer's lambda expression in a Qt-based class can cause a crash if the object is deleted while there is an event still in the Qt Event loop.  The solution is to use a QPointer instead so that it can be tested for nullptr.

### Adding Observers to Views
* Observation for attribute creation, modification, and expungement have been added to qtAttributeView.
* Observation for attribute modification has been added to qtInstanceView

### Changes to qtGroupItem
* The first Column is no longer marked with 1 for extensible groups.
* Fixed issue with updating extensible qtGroupItems due to the number of columns being set to 0 instead of 1

### API Changes
These changes were made to help simplify/cleanup the qtView infrastructure.  There were several places where onShowCategory() was being called in order to update the UI.  This resulted in confusion as to the role of the method.  In many cases these calls have been replaced with updateUI.

* **qtBaseView::updateViewUI - has been removed.** It was not being used.
* **qtBaseAttributeView::updateAttributeData - has been removed.** This method's role was to update the attribute content of a View.  You should now call updateUI() instead.
* qtBaseAttributeView no longer overrides updateUI()

### Tracking Changes in Analysis Configuration Attributes
Attributes that are deleted, created, or modified are now checked to see if they represent Analysis Configurations.  The configuration combobox is then updated appropriately.

### Other Changes
* qtItem::updateItemData has been made public so that qtItems can be undated when their underlying attribute items are external changed.
* qtGroupItem will now adjust the subgroup table's height based on the number of rows it contains.
* qtGroupItem now supports attribute::GroupItem::prepend method. This is controlled by specifying InsertMode="Prepend" attribute in the appropriate ItemView as shown below:

```xml
        <Att Name="numerics-att" Type="numerics">
          <ItemViews>
            <View Item="velocity-group" Type="Default" InsertMode="Prepend"/>
         </ItemViews>
    </Att
```
For a complete example see data/attribute/attribute_collection/ConfigurationTest.sbt.

* Extensible items that provided default values would cause qtInputsItem to crash once more entries were added than there were defaults. This has been fixed to repeat the first default value for all new entries that do not have a valid default.

## Changes to the ParaView UI Subsystem

### Widgets

Now, all of the 3-d widgets will be hidden when their Qt partner-widget
is not visible (and visibility will be restored when this changes).

The box widget (pqSMTKBoxItemWidget) now supports a binding that allows
the visibility of the widget to be mapped to a discrete-valued string
item with enumerants "active" and "inactive".

#### Fixed Issues
+ An issue caused by widgets being asked to update from item contents during user interaction was fixed.
  This fix requires a change to all subclasses of pqSMTKAttributeItemWidget; any override of either
  `updateWidgetFromItem()` or `updateItemFromWidget()` should be renamed to `updateWidgetFromItemInternal()`
  or `updateItemFromWidgetInternal()` (respectively).
  Now, the non-`Internal()` methods update a new internal `m_p->m_state` variable so the widget does not
  attempt updates caused by itself.


### Subtractive UI

You may now subtract basic ParaView UI elements (QActions
such as toolbar buttons and menu items) by calling methods
on the pqSMTKSubtractiveUI class during your plugin's
initialization (or at later times as needed).

Note that

+ You should not attempt to remove UI elements added by
  other plugins since the order in which plugins are
  loaded is unspecified.
+ You may disable and re-enable items in response to
  changes in the interface (e.g., the CMB "post-processing"
  plugin, which disables the "Sources" and "Filters" menu
  items at startup but re-enables them when users enter
  post-processing mode.

### Selection

Operations are now used to translate VTK/ParaView selections
into SMTK selections. See the user guide for details.
This change was made to support mesh and instance subset selections.

### Operators
#### Deprecated Operatiors
These ParaView operators are no longer supported:

* smtkSaveModelView
* smtkExportModelView

### Collapse VTK sources & representations to use one port instead of three

Originally, VTK sources for SMTK resources had three output sources:
one for components, one for instance prototypes and one for instance
placements. A single instance of the consuming
vtkSMTKResourceRepresentation would then ideally connect to these
three ports. ParaView's default behavior resulted in the creation of
an instance of the representation class for each port, however,
resulting in duplicate rendering of instance prototypes as components
(in addition to other rendering artifacts, such as z-fighting).

The new design passes component multiblocks, instance prototypes and
instance placements via a single port as a multiblock with three
blocks. The consuming representation now extracts each of the three
blocks and renders them appropriately.

### Cache VTK data objects for improved performance

The tessellation generation number (stored as an integer property) is
now used to determine whether a cached VTK data object can be used
instead of generating a new one from a model tessellation or mesh set.
This reduces the time between operation completion and rendering for
large geometries.

### Preference to skip modified resource dialog

When a user closes a modified resource or exits the application with a
resource that is modified, a dialog is presented which asks the user
to save, discard, or cancel. Add a checkbox to the dialog that sets
the default response to this dialog, to save or discard the resource.

If the user changes their mind, the preference can be changed in the
Settings dialog.



## Test Changes

- Removed one problematic test, `displayMultiBlockModel-simple`, that used an outdated input file and rendered differently on some dashboard systems.
- Added `knee.ex2` exodus test data, used by ModelBuilder tests for now.
- If we are running tests, don't display the "Your data is modified, save changes?" dialog on exit.
- Plugin (contract) tests now have a timeout of 600 seconds to allow more time for cloning and building a repository.
- fix the smtkAssignColorsView pallet choose dialog handling so it is testable with xml tests.
