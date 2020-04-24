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
```
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

### Changes to qtBaseView
* Added a virtual isValid method to indicate if a view is valid
* Added a modified signal so that qtBaseViews can indicate they have been changed


### qtAttributeView Changes
* There is now a splitter between the attribute editing area and the association information area.
* Removed the old view by property mechanism to help simplify the code
* qtAttributeView and qtAssociatioView now have virtual methods to create their association widgets that can get overridden - in the future they should use a widget factor to fetch it so you wouldn't have to create a new class to use a different association widget
* XML Option RequireAllAssociated="true" will now display the qtAssociationWidget even if no attributes exists and display a warning if there are persistent objects that match the definition requirements but are not associated to any attribute.
* Added XML Option DisableNameField="true" that indicates that the attribute's name should not be changed.


### qtUIManager Changes
* There is now a method to return the size of a string based on the font being used

### qtInputItem Changes
* If the space reserved for the label width is less than 1/2 the space required. The size hint is ignored and enough space for the entire label is used.

### qtGroupItem Changes
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

### Changes to qtGroupView
* GroupBox Style icon has been changed from a check box to a closed/expand icon pair.  This change reduces confusion between optional items and viewing control widgets.
* Tabbed Group Views now show indicate invalid children views in their tabs using the alert icon

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
