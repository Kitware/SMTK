## I/O Changes

### smtk::io::Logger is now thread-safe
The class now does a mutex lock when modifying or accessing its records or its underlying stream.  Care must be taking when redirecting the logger's stream to avoid deadlocks.  For example using smtk::extension::qtEmittingStringBuffer, you should make sure to use Qt::QueuedConnection when doing a QObject::connect to the buffer's flush signal.  See smtk/extension/qt/cxx/testing/UnitTestEmittingStringBuffer.{h,cxx} for an example.

### New Attribute Resource and SBT formats (4.0)
We have now create version 4.0 for both attribute SBT and attribute SMTK files.  This will now be the default in terms of writing out attribute information.  SMTK will still support reading in Versions 1, 2, and 3.

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
