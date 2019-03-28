#New View Type - Associations
This view has the same syntax as an Attribute View but only allows the user to change the association information of the attribute resulting in taking up less screen Real Estate

#Changes to Attribute View
* added a new XML attribute "HideAssociations".  If set to true the view will not display the association editing widget save screen Real Estate
* If there is only one type of attribute being created/modified then the type column is no longer displayed
* For the time being the view by property  feature has been disabled until we can decide on whether it is useful and if so, what is the best way to display the information.
* The column "Attribute" has been renamed to "Name"

#New View Type - Analysis
An Analysis View is a specialized view for choosing the types of analyses the user wants to perform.  These choices are persistent and can be used by an export operation instead of having the operator ask the user what types of analyses should be performed.

Unlike other views the Analysis View will construct both an Attribute Definition and corresponding Attribute when needed.  The Attribute Definition is based on the Analysis Information stored in the Attribute Resource.  Any Analysis that is referred to by another will be represented as a Group Item.  All other Analyses will be represented as a Void Item.

The View also controls which categories are permitted to be displayed and/or selected.  The set is union of all of the selected Analyses' categories.

The following is an example of a Analysis View:

```xml
 <View Type="Analysis" Title="Analysis" AnalysisAttributeName="truchasAnalysis"
 AnalysisAttributeType="truchasAnalysisDefinition">
</View>
```

  * AnalysisAttributeType is the name of the Attribute Definition the view will create to represent the Analysis Structure (if needed)
  * AnalysisAttributeName is the name of the Attribute the view will create to represent the Analysis  (if needed)

#Changes to Group View
* View no longer displays empty tabs

# Changes to BaseView
* Added the concept of top level categories that represents a set of categories (that can be a subset of those defined in the attribute resource) that can be used to display or filter attribute information.

#Changes to UIManager
* Added the ability to enable/disable category filtering
* Added support for top-level categories

#ReadOnly View Items
Added a new ReadOnly Option to Item Views.  In the following example the item, absolute-zero, in the attribute physical-constants has been made read only.  The current implementation disables the widgets defined by the read only item from being  modified but will still display them.

```xml
    <View Type="Instanced" Title="Global Constants">
      <InstancedAttributes>
        <Att Name="physics" Type="physics">
          <ItemViews>
            <View Item="fluid-body-force" Type="Default" FixedWidth="0"/>
          </ItemViews>
        </Att>
        <Att Name="physical-constants" Type="physical-constants">
          <ItemViews>
            <View Item="absolute-zero" Type="Default" ReadOnly="true" FixedWidth="50/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
```
#Changes to Displaying Items
* Extensible Group Items know display "Add Row" instead of "Add sub group" - this label can be changed using an ItemView with the XML attribute : ExtensibleLabel
* Added a FixedWidth Option for Double and Int ItemViews as also shown in the above example - **Note: setting the fixed width to 0 means there is no fixed width.**
