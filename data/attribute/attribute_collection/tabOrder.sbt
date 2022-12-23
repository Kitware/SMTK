<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">

  <Definitions>
    <AttDef Type="SingleInts">
      <ItemDefinitions>
        <Int Name="Before1"><DefaultValue>1</DefaultValue></Int>
        <Int Name="Before2"><DefaultValue>2</DefaultValue></Int>
        <Group Name="SingleInts" Extensible="true">
          <ItemDefinitions>
            <Int Name="First"><DefaultValue>1</DefaultValue></Int>
            <Int Name="Second">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="One">1</Value>
                <Value Enum="Two">2</Value>
                <Value Enum="Three">3</Value>
              </DiscreteInfo>
            </Int>
            <Int Name="Third"><DefaultValue>3</DefaultValue></Int>
          </ItemDefinitions>
        </Group>
        <Int Name="After1"><DefaultValue>1</DefaultValue></Int>
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
        <View Title="SingleInts" />
      </Views>
    </View>

    <View Type="Instanced" Title="SingleInts" Label="Single Ints">
      <InstancedAttributes>
        <Att Type="SingleInts" Name="SingleInts" />
      </InstancedAttributes>
    </View>

  </Views>
</SMTK_AttributeResource>
