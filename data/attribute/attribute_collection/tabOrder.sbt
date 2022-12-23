<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">

  <Definitions>
    <AttDef Type="ExtensibleGroup">
      <ItemDefinitions>
        <Int Name="Before1"><DefaultValue>1</DefaultValue></Int>
        <Double Name="Before2"><DefaultValue>3.14159</DefaultValue></Double>
        <Group Name="ExtensibleGroup" Extensible="true">
          <ItemDefinitions>
            <Int Name="First"><DefaultValue>1</DefaultValue></Int>
            <Int Name="Second">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="One">1</Value>
                <Value Enum="Two">2</Value>
                <Value Enum="Three">3</Value>
              </DiscreteInfo>
            </Int>
            <Double Name="Third" NumberOfRequiredValues="2">
              <ComponentLabels>
                <Label>Label1</Label>
                <Label>Label2</Label>
              </ComponentLabels>
              <DefaultValue>3.33,6.66</DefaultValue>
            </Double>
            <String Name="text"><DefaultValue>Fourth</DefaultValue></String>
          </ItemDefinitions>
        </Group>
        <String Name="After1"><DefaultValue>String data</DefaultValue></String>
        <Int Name="After2"><DefaultValue>2</DefaultValue></Int>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="Example" />
   </Definitions>

  <Attributes>
    <Att Type="Example" Name="Example" />
  </Attributes>

  <Views>
    <View Type="Group" Title="Tab Order Examples" TabPosition="North" TopLevel="true"
          FilterByAdvanceLevel="false" FilterByCategory="false"
          UseScrollingContainer="false">
      <Views>
        <View Title="ExtensibleGroup" />
      </Views>
    </View>

    <View Type="Instanced" Title="ExtensibleGroup" Label="Extensible Group">
      <InstancedAttributes>
        <Att Type="ExtensibleGroup" Name="ExtensibleGroup" />
      </InstancedAttributes>
    </View>

  </Views>
</SMTK_AttributeResource>
