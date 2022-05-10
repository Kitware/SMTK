<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">
  <AdvanceLevels>
    <Level Label="0">0</Level>
    <Level Label="1">1</Level>
    <Level Label="2">2</Level>
  </AdvanceLevels>
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <AttDef Type="A" Label="A" BaseType="" >
      <ItemDefinitions>
        <Group Name="g0" AdvanceReadLevel="0" NumberOfRequiredGroups="1" Optional="true">
          <ItemDefinitions>
            <String Name="stringAccessLevel1" AdvanceLevel="1" />
            <String Name="stringAccessLevel2" AdvanceLevel="2" />
          </ItemDefinitions>
        </Group>
        <Group Name="g1" AdvanceReadLevel="0" NumberOfRequiredGroups="1" Extensible="true" Optional="true">
          <ItemDefinitions>
            <String Name="stringAccessLevel1" AdvanceLevel="1" />
            <String Name="stringAccessLevel2" AdvanceLevel="2" />
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Attribute" Title="Empty GroupItem Test" TopLevel="true" DisableTopButtons="false">
      <AttributeTypes>
        <Att Type="A"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
