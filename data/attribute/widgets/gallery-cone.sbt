<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Group Name="cone" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="point 0" NumberOfRequiredValues="3"/>
            <Double Name="point 1" NumberOfRequiredValues="3"/>
            <Double Name="radius 0"/>
            <Double Name="radius 1"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Cone Widget Example" Type="Example">
      <Items>
        <Group Name="cone" NumberOfGroups="1">
          <GroupClusters>
            <Cluster Ith="0">
              <Double Name="point 0">
                <Values>
                  <Val Ith="0">0.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">0.0</Val>
                </Values>
              </Double>
              <Double Name="point 1">
                <Values>
                  <Val Ith="0">1.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">0.0</Val>
                </Values>
              </Double>
              <Double Name="radius 0">
                <Values>
                  <Val Ith="0">0.5</Val>
                </Values>
              </Double>
              <Double Name="radius 1">
                <Values>
                  <Val Ith="0">0.0</Val>
                </Values>
              </Double>
            </Cluster>
          </GroupClusters>
        </Group>
      </Items>
    </Att>
  </Attributes>
  <Views>
    <View
      Type="Instanced"
      Title="Example"
      TopLevel="true"
      FilterByAdvanceLevel="false"
      FilterByCategoryMode="false"
    >
      <InstancedAttributes>
        <Att Type="Example" Name="Cone Widget Example"
        >
          <ItemViews>
            <View Item="cone" Type="Cone" BottomPoint="point 0" TopPoint="point 1" BottomRadius="radius 0" TopRadius="radius 1" ShowControls="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
