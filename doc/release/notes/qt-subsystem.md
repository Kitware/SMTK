## Changes to the View System and Qt Extensions

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
