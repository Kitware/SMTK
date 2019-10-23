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

### Other Changes
* qtItem::updateItemData has been made public so that qtItems can be undated when their underlying attribute items are external changed.
* qtGroupItem will now adjust the subgroup table's height based on the number of rows it contains.
