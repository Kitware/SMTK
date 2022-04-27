<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Group Name="coordinate frame" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="origin" NumberOfRequiredValues="3"/>
            <Double Name="x axis" NumberOfRequiredValues="3"/>
            <Double Name="y axis" NumberOfRequiredValues="3"/>
            <Double Name="z axis" NumberOfRequiredValues="3"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Coordinate Frame Widget Example" Type="Example">
      <Items>
        <Group Name="coordinate frame" NumberOfGroups="1">
          <GroupClusters>
            <Cluster Ith="0">
              <Double Name="origin">
                <Values>
                  <Val Ith="0">0.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">0.0</Val>
                </Values>
              </Double>
              <Double Name="x axis">
                <Values>
                  <Val Ith="0">1.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">0.0</Val>
                </Values>
              </Double>
              <Double Name="y axis">
                <Values>
                  <Val Ith="0">0.0</Val>
                  <Val Ith="1">1.0</Val>
                  <Val Ith="2">0.0</Val>
                </Values>
              </Double>
              <Double Name="z axis">
                <Values>
                  <Val Ith="0">0.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">1.0</Val>
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
        <Att Type="Example" Name="Coordinate Frame Widget Example"
        >
          <ItemViews>
            <View Item="coordinate frame" Type="CoordinateFrame"
              Origin="origin"
              XAxis="x axis"
              YAxis="y axis"
              ZAxis="z axis"
              ShowControls="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
