<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Double Name="point" NumberOfRequiredValues="3"/>
        <!--
        <Group Name="handled" NumberOfRequiredGroups="1" Optional="true">
          <ItemDefinitions>
            <Double Name="coords" NumberOfRequiredValues="3"/>
            <String Name="cstate">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Active">active</Value>
                <Value Enum="Visible">visible</Value>
                <Value Enum="Inactive">inactive</Value>
              </DiscreteInfo>
            </String>
          </ItemDefinitions>
        </Group>
        -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Point Widget Example" Type="Example">
      <Items>
        <Double Name="point">
          <Values>
            <Val Ith="0">0.035</Val>
            <Val Ith="1">0.035</Val>
            <Val Ith="2">0.125</Val>
          </Values>
        </Double>
        <!--
        <Group Name="handled" Enabled="true" NumberOfGroups="1">
          <GroupClusters>
            <Cluster Ith="0">
              <Double Name="coords">
                <Values>
                  <Val Ith="0">0.5</Val>
                  <Val Ith="1">0.5</Val>
                  <Val Ith="2">0.125</Val>
                </Values>
              </Double>
              <String Name="cstate">
                <Values>
                  <Val Ith="0">active</Val>
                </Values>
              </String>
            </Cluster>
          </GroupClusters>
          </Group>
        -->
      </Items>
    </Att>
  </Attributes>
  <Views>
    <View
      Type="Instanced"
      Title="Example"
      TopLevel="true"
      FilterByAdvanceLevel="false"
      FilterByCategoryMode="off"
    >
      <InstancedAttributes>
        <Att Type="Example" Name="Point Widget Example"
        >
          <ItemViews>
            <View Item="point" Type="Point" ShowControls="true"/>
            <!-- View Item="handled" Type="Point" Coords="coords" Control="cstate" ShowControls="true"/ -->
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
