<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <String Name="String Item">
          <DefaultValue>Any printable text</DefaultValue>
        </String>
        <Double Name="Double Item">
          <DefaultValue>3.14159</DefaultValue>
        </Double>
        <Double Name="Advanced (Double) Item" AdvanceLevel="1">
          <DefaultValue>2.71828</DefaultValue>
        </Double>
        <Int Name="Integer Item">
          <DefaultValue>42</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Attribute" Title="Example" TopLevel="true" FilterByCategory="false">
      <AttributeTypes>
        <Att Type="Example" />
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
