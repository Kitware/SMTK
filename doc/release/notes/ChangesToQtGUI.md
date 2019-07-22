Note any non-backward compatible changes are in ***bold italics***.

## Changes to the View System and Qt Extensions

### AttributeItemInfo has been renamed
The new name is qtAttributeItemInfo and it is now located in its own header and cxx files instead of qtItem.h and qtItem.cxx.  **To update existing code all you need do is a name replace.**  qtItem.h includes qtAttributeItemInfo.h so no additional includes are needed.

### Supporting Item Paths for ItemViews
You can now refer to an item via its path relative to its owning Attribute and uses UNIX style.  Item names that could be used as variable names in C++ are supported and have been extended to also include . and -.  The following show several examples:

```xml
  <Views>
    <View Type="Instanced" Title="Grammar Test" TopLevel="true">
      <InstancedAttributes>
        <Att Name="Test Attribute" Type="test">
          <ItemViews>
            <View Item="a" Type="Default" Option="SpinBox"/>
            <View Path="/b" ReadOnly="true"/>
            <View Path="/c/d" FixedWidth="20"/>
            <View Path="/c/e" Option="SpinBox"/>
            <View Path="/f/f-a" Option="SpinBox"/>
            <View Path="/f/f-b" FixedWidth="10"/>
            <View Path="/f/f-c/f.d" FixedWidth="0"/>
            <View Path="/f/f-c/f.e" Option="SpinBox"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
```
In the case of extensible Group Items, the item view style refers to subgroups.  For a complete example, please see viewPathTest.sbt that is located in data/attribute/attribute_collection.

### Changes to qtItem
* **Removed passAdvanceCheck method** - In the qtItem classes, to determine if an item should be viewed, the code was either calling qtBaseView::displayItem check or a combination of qtUIManager::passItemCategoryCheck and this method.  Since a view can have its own set of filtering rules, having this method just confuses the developer as to how to determine if the item should be displayed.
* Added removedChildItem(..) method - this removes a child qtItem from the object and calls deleteLater() on it.
* **addChildItem(..), clearChildItems() and childItems() methods are now protected.**  They use to be public which was a mistake.

### Changes to qtFileItem
* Invalidity calculation now takes into consideration the ShouldExists hint associated with the item's definition.
* Combobox now only sets the item's value when the user finishes entering the filename instead of every keystroke.

### Changes to qtInputItem
* Added a forceUpdate method - this always forces the object to act as if the underlying item was modified.  Used mainly by helper classes like qtDiscreteValueEditor.

### Added qtReferenceItemComboBox
This is a new type of qtItem used to create a simple ComboBox UI for setting a qtReferenceItem.  It also supports the ability to restrict its possible values to those objects associated with the item's attribute.

* In the case that the item is not set, "Please Select" is now displayed in red.

### Changes to qtDiscreteValueEditor
* The modified signal from the corresponding qtInputItem is no longer sent when the underlying ValueItem is modified.  It is now sent after the Editor's internal widgets have been appropriately updated.

### Changes to qtBaseView
* The displayItem test now calls 2 new methods categoryTest and advanceLevelTest.  This makes it easier for derived classes to override the filtering behavior

### Changes to qtAnalysisView
* Now supports Exclusive property for Analysis and Top Level Analyses
* The order of displaying analyses is no longer resorted alphabetically and will be displayed in the order they were defined.
* Overrides the categoryTest method to return always true since its data should never be filtered based on categories.

### Changes to qtAttributeView
* The view now displays an alert icon next to attributes that are in an invalid state

### Changes to qtUIManager
* The set of categories is being passed to setToLevelCategories is now compared to what was already set.

### Changes to qtAssociationWidget
* Added the ability to ignore a resource when determining which objects can be associated with an attribute.  The main use case is when refreshing the widget because a resource is about to be removed from the system.  We don't want it to contribute to the calculation.
* The widget now shows an alert icon if the associations are not valid.

### Bug Fixes
* qtAnalysisView, qtAttributeView, qtInstancedView, qtModelEntityView and qtSelectorView now properly deletes any qtAttributes they create
* qtGroupItem now properly delete any children qtItems it creates.  It was only deleting children if it was extensible.
