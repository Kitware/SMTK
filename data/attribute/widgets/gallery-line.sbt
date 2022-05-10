<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Group Name="line" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="point 0" NumberOfRequiredValues="3"/>
            <Double Name="point 1" NumberOfRequiredValues="3"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Line Widget Example" Type="Example">
      <Items>
        <Group Name="line" NumberOfGroups="1">
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
        <Att Type="Example" Name="Line Widget Example"
        >
          <ItemViews>
            <View Item="line" Type="Line" Point1="point 0" Point2="point 1" ShowControls="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
