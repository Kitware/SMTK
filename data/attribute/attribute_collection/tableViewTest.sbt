<?xml version="1.0"?>
<SMTK_AttributeResource
  Version="8"
  DisplayHint="false"
  TemplateType="common"
  TemplateVersion="1"
>

<Definitions>

  <AttDef Type="TestDef" Label="Test Definition" >
    <ItemDefinitions>
      <Double Name="d0" Label="D0">
        <DefaultValue>5.5</DefaultValue>
        <RangeInfo>
          <Min Inclusive="false">0</Min>
          <Max Exclusive="false">10</Max>
        </RangeInfo>
        <BriefDescription>Brief Desacription</BriefDescription>
      </Double>
      <Double Name="Velocity" Label="Velocity" NumberOfRequiredValues="3">
        <ComponentLabels>
          <Label>x=</Label>
          <Label>y=</Label>
          <Label>z=</Label>
        </ComponentLabels>
      </Double>
      <Int Name="time" Label="Time" NumberOfRequiredValues="1">
        <DefaultValue>10</DefaultValue>
      </Int>
      <Int Name="pressure" Label="Pressure" NumberOfRequiredValues="1">
        <DefaultValue>20</DefaultValue>
        <RangeInfo>
          <Min Exclusive="false">0</Min>
          <Max Inclusive="false">100</Max>
        </RangeInfo>

      </Int>
      <String Name="StringItem2" Label="StringItem2" NumberOfRequiredValues="1"/>
      <Void Name="enable" Label="Enable" Optional="true"/>
      <String Name="btype" Label="BC">
        <ChildrenDefinitions>
          <String Name="wtype" Label="Wall">
            <ChildrenDefinitions>
              <Double Name="roughnessHeight" Label="Roughness Height (Ks)" Units="m">
                <RangeInfo>
                  <Min Inclusive="true">0.0</Min>
                </RangeInfo>
              </Double>
            </ChildrenDefinitions>
            <DiscreteInfo DefaultIndex="0">
              <Structure>
                <Value Enum="Slip">slip</Value>
              </Structure>
              <Structure>
                <Value Enum="No Slip">noslip</Value>
                <Items>
                  <Item>roughnessHeight</Item>
                </Items>
              </Structure>
            </DiscreteInfo>
          </String>
          <String Name="ptype" Label="Patch">
            <ChildrenDefinitions>
              <Group Name="velProfile" Label="Volumetric Flow Rate" Extensible="true" NumberOfRequiredGroups="1">
                <ItemDefinitions>
                  <Double Name="info" Label="Info" NumberOfRequiredValues="2">
                    <ComponentLabels>
                      <Label>Time:</Label>
                      <Label>Flow:</Label>
                    </ComponentLabels>
                  </Double>
                </ItemDefinitions>
              </Group>
              <Double Name="value" Label="Value">
                <RangeInfo>
                  <Min Inclusive="true">0.0</Min>
                </RangeInfo>
              </Double>
            </ChildrenDefinitions>
            <DiscreteInfo DefaultIndex="0">
              <Structure>
                <Value Enum="Rate">flow</Value>
                <Items>
                  <Item>velProfile</Item>
                </Items>
              </Structure>
              <Structure>
                <Value Enum="Tail Water Elevation" Units="m">wp</Value>
                <Items>
                  <Item>value</Item>
                </Items>
              </Structure>
              <Structure>
                <Value Enum="Standard Atmosphere">ap</Value>
              </Structure>
            </DiscreteInfo>
          </String>
        </ChildrenDefinitions>
        <DiscreteInfo DefaultIndex="0">
          <Structure>
            <Value Enum="Wall">wall</Value>
            <Items>
              <Item>wtype</Item>
            </Items>
          </Structure>
          <Structure>
            <Value Enum="Patch">patch</Value>
            <Items>
              <Item>ptype</Item>
            </Items>
          </Structure>
        </DiscreteInfo>
      </String>
    </ItemDefinitions>
  </AttDef>
</Definitions>

  <Views>
    <View Type="AttributeTable" Title="Main" TopLevel="true"  DeleteOp="foo" HideInactiveChildren="true">
      <AttributeTypes>
        <Att Type="TestDef">
        </Att>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
