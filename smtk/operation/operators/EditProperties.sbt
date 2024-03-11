<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "EditProperties" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="edit properties" Label="freeform properties" BaseType="operation">
      <AssociationsDef Name="entities" NumberOfRequiredValues="1" Extensible="true">
        <Accepts>
          <Resource Name="smtk::resource::Resource"/>
          <Resource Name="smtk::resource::Resource" Filter="*"/>
        </Accepts>
      </AssociationsDef>
      <BriefDescription>Set (or remove) an attribute value on a component.</BriefDescription>
      <DetailedDescription>
        Set (or remove) an attribute value on a component.
        The string, integer, and floating-point values are all optional.
        Any combination may be specified.
        All that are specified are set; those unspecified (i.e., those with
        zero values of the given type) are removed.

        For example, specifying both a string and an integer value for
        the "foo" attribute would set those values in the resource's
        string and integer maps while removing "foo" from the associated
        entities' floating-point map.
      </DetailedDescription>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1">
          <BriefDescription>The name of the parameter to be modified/created/deleted.</BriefDescription>
        </String>
        <Int Name="type">
          <BriefDescription>The parameter type being modified.</BriefDescription>
          <ChildrenDefinitions>
            <Double Name="float value">
              <BriefDescription>Floating-point value(s) of the attribute.</BriefDescription>
            </Double>
            <String Name="string value">
              <BriefDescription>String value(s) of the attribute.</BriefDescription>
            </String>
            <Int Name="integer value">
              <BriefDescription>Integer value(s) of the attribute.</BriefDescription>
            </Int>
            <Group Name="Coordinate Frame" Label=" ">
              <ItemDefinitions>
                <Double Name="Origin" Label="origin" NumberOfRequiredValues="3">
                  <DefaultValue>0, 0, 0</DefaultValue>
                  <BriefDescription>Origin point of the coordinate frame.</BriefDescription>
                </Double>
                <Double Name="XAxis" Label="x axis" NumberOfRequiredValues="3">
                  <DefaultValue>1, 0, 0</DefaultValue>
                  <BriefDescription>XAxis vector of the coordinate frame.</BriefDescription>
                </Double>
                <Double Name="YAxis" Label="y axis" NumberOfRequiredValues="3">
                  <DefaultValue>0, 1, 0</DefaultValue>
                  <BriefDescription>YAxis vector of the coordinate frame.</BriefDescription>
                </Double>
                <Double Name="ZAxis" Label="z axis" NumberOfRequiredValues="3">
                  <DefaultValue>0, 0, 1</DefaultValue>
                  <BriefDescription>ZAxis vector of the coordinate frame.</BriefDescription>
                </Double>
                <Reference Name="Parent" Label="parent" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
                  <Accepts><Resource Name="smtk::resource::Resource" Filter="any"/></Accepts>
                  <BriefDescription>Reference to  the parent component.</BriefDescription>
                </Reference>
              </ItemDefinitions>
            </Group>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="String">0</Value>
              <Items>
                <Item>string value</Item>
              </Items>
            </Structure>
             <Structure>
              <Value Enum="Floating Point">1</Value>
              <Items>
                <Item>float value</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Integer">2</Value>
              <Items>
                <Item>integer value</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Coordinate System">3</Value>
              <Items>
                <Item>Coordinate Frame</Item>
              </Items>
            </Structure>
         </DiscreteInfo>
        </Int>
        <Void Name="remove" Optional="true" IsEnabledByDefault="false">
            <BriefDescription>If enabled, indicates that the property should be removed.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(edit properties)" BaseType="result"/>
  </Definitions>
  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME EditPropertiesView ...)
      -->
    <View Type="smtkEditPropertiesView" Title="freeform properties"
      FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="true">
      <Description>
        Enter an attribute name and type to add or remove.
        The list beneath the editor shows properties currently present on the selected components.
      </Description>
      <AttributeTypes>
        <Att Type="edit properties"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
