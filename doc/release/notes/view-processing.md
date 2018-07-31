# Changes to Views and View Processing
## Model Entity Centric Views
There are use cases where every model entity needs to have an attribute associated with it.  For example a simulation may require every surface to have a boundary condition associated with it.  It should be easy to glance at the presented view and determine which surfaces do and do not have the desired association. **qtModelEntityAttribute** is one view renderer that provides this type of functionality. Note that each model entity will have its own attribute associated with it (no sharing). Below is a XML Example:

```xml
    <View Type="ModelEntity" Title="Surface Properties" ModelEntityFilter="f">
      <AttributeTypes>
        <Att Type="SurfaceProperty" />
      </AttributeTypes>
    </View>
```
In this case all Model Faces will be displayed and will have an attribute derived from SurfaceProperty associated with it.

## Supporting Empty Views in a Tiled Group View
+ If a child view is not displaying any attribute items the owning tiled group view will no longer display the title of the child view.

## Item View Support
It is now possible to control how attribute items get presented in the Qt interface.  You can now specify an ItemView section within the Att block of the XML or JSON.  Below is a XML example:

```xml
    <View Type="Instanced" Title="Frequency Information">
      <InstancedAttributes>
        <Att Name="EigenSolver" Type="FrequencyInfo">
          <ItemViews>
            <View Item="NumEigenvalues" Type="Default" Option="SpinBox"/>
            <View Item="FrequencyShift" Option="SpinBox" StepSize="100" Decimals="0"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
```
+ The XML attribute "Item"  must match the name of the item in the SMTK attribute.  For example, NumEigenvalues is an item in a FrequencyInfo attribute.
+ The XML attribute "Type" corresponds to the Qt item class used to present the item.  If not specified it is assumed to be Default
+ The rest of the information is then processed by the Qt item class.  For example the Default qtItem class for double items is qtDoubleItem which supports the following:
 + Option="SpinBox" - use a qtDoubleSpinBox to manipulate the item.  You can specify the number of decimals and set size.
 + Option="lineEdit" (Default) - use a qtLineEdit widget to manipulate the item.
+ ItemViews are currently supported in the following views:
 + Type="Attribute" (qtAttributeView)
 + Type="Instanced" (qtInstancedView)
 + Type="ModelEntity" (qtModelEntityAttributeView)
 + Type="Operator" (qtOperatorView)

## General Code Changes
+ qtAttributeItemWidgetFactory was removed - all functionality has been added to qtUIManager
+ All icons have been moved to icons subdirectory
+ All qtItem classes now take in a smtk::extension::AttributeItemInfo object that is used to define the instance
+ Added qtDoubleItem, qtIntItem, qtStringItem classes that use the qtInputsItem class
+ Added qtFileItem and qtDirectory classes that use the qtFileSystemItem class.
