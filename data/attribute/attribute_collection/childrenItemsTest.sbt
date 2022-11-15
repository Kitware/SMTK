<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6" >
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="GeneralAttributeDef" Unique="false">
      <ItemDefinitions>
        <Int Name="TEMPORAL" Label="Time" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Seconds">0</Value>
            <Value Enum="Minutes">1</Value>
            <Value Enum="Hours">2</Value>
            <Value Enum="Days">3</Value>
          </DiscreteInfo>
        </Int>
        <Int Name="IntItem2" Label="Simple Integer" NumberOfRequiredValues="1">
          <DefaultValue>10</DefaultValue>
        </Int>
        <Double Name="DoubleItem1" Label="Double Exp" NumberOfRequiredValues="1" >
          <ExpressionType>ExpDef</ExpressionType>
        </Double>
        <Double Name="DoubleItem2" Label="Discrete Double" NumberOfRequiredValues="1">
          <ChildrenDefinitions>
            <Double Name="Child1" Label="Child1" NumberOfRequiredValues="1">
            </Double>
            <Int Name="Child2" Label="Child2" NumberOfRequiredValues="1">
            </Int>
            <String Name="Child3" Label="Child3" NumberOfRequiredValues="1">
            </String>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="A">0</Value>
              <Items>
                <Item>Child1</Item>
                <Item>Child3</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="B">1</Value>
              <Items>
                <Item>Child3</Item>
                <Item>Child2</Item>
              </Items>
            </Structure>
            <Value Enum="C">2</Value>
          </DiscreteInfo>
        </Double>
       <Double Name="velocity" Label="Velocity" NumberOfRequiredValues="3"  Units="m/s">
          <BriefDescription>Coordinates of the probe location</BriefDescription>
          <ComponentLabels>
            <Label>x=</Label>
            <Label>y=</Label>
            <Label>z=</Label>
          </ComponentLabels>
        </Double>
        <Void Name="VoidItem" Label="Option 1" Optional="true" IsEnabledByDefault="false">
        </Void>
        <String Name="StringItem1" Label="StringItem1" NumberOfRequiredValues="1" MultipleLines="true">
        </String>
        <String Name="StringItem2" Label="StringItem2" NumberOfRequiredValues="1">
          <DefaultValue>Default</DefaultValue>
        </String>
        <Directory Name="DirectoryItem" Label="DirectoryItem" NumberOfRequiredValues="1" ShouldExist="true" ShouldBeRelative="true">
        </Directory>
        <File Name="FileItem" Label="FileItem" NumberOfRequiredValues="1" ShouldBeRelative="true">
        </File>
         <Group Name="GroupItem" Label="GroupItem" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <File Name="File1" Label="File1" NumberOfRequiredValues="1">
            </File>
            <Group Name="SubGroup" Label="SubGroup" NumberOfRequiredGroups="1">
              <ItemDefinitions>
                <String Name="GroupString" Label="GroupString" NumberOfRequiredValues="1">
                  <DefaultValue>Something Cool</DefaultValue>
                </String>
              </ItemDefinitions>
            </Group>
          </ItemDefinitions>
        </Group>
         <Group Name="ExtendGroupItem" Label="GroupItem 2" NumberOfRequiredGroups="0" Extensible="true">
          <ItemDefinitions>
            <Int Name="a" Label="a" NumberOfRequiredValues="1">
            </Int>
            <Double Name="b" Label="b" NumberOfRequiredValues="1">
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="ExpDef" Label="ExpDef" Unique="false">
      <BriefDescription>Sample Expression</BriefDescription>
      <DetailedDescription>Sample Expression for testing
There is not much here!</DetailedDescription>
      <ItemDefinitions>
        <String Name="Expression String" Label="Expression String" NumberOfRequiredValues="1">
          <DefaultValue>sample</DefaultValue>
        </String>
        <String Name="Aux String" Label="Aux String" NumberOfRequiredValues="1">
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <!--********** Workflow Views ***********-->
  <Views>
    <View Type="Group" Name="TopLevel" FilterByAdvanceLevel="true"  Style="Tiled" TopLevel="true">
      <Views>
        <View Title="Test" />
        <View Title="Exp" />
      </Views>
    </View>
    <View Type="Instanced" Name="Test">
      <InstancedAttributes>
        <Att Name="testAtt" Type="GeneralAttributeDef" />
      </InstancedAttributes>
    </View>
    <View Type="Instanced" Name="Exp">
      <InstancedAttributes>
        <Att Name="Exp1" Type="ExpDef" />
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
