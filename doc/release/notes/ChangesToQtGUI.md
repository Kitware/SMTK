Note any non-backward compatible changes are in ***bold italics***.

## Changes to the View System and Qt Extensions

### AttributeItemInfo has been renamed
The new name is qtAttributeItemInfo and it is now located in its own header and cxx files instead of qtItem.h and qtItem.cxx.  To update existing code all you need do is a name replace.  qtItem.h includes qtAttributeItemInfo.h so no additional includes are needed.

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
