<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Group Name="plane" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="origin" NumberOfRequiredValues="3"/>
            <Double Name="normal" NumberOfRequiredValues="3"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Plane Widget Example" Type="Example">
      <Items>
        <Group Name="plane" NumberOfGroups="1">
          <GroupClusters>
            <Cluster Ith="0">
              <Double Name="origin">
                <Values>
                  <Val Ith="0">0.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">0.0</Val>
                </Values>
              </Double>
              <Double Name="normal">
                <Values>
                  <Val Ith="0">1.0</Val>
                  <Val Ith="1">1.0</Val>
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
        <Att Type="Example" Name="Plane Widget Example"
        >
          <ItemViews>
            <View Item="plane" Type="Plane" Origin="origin" Normal="normal" ShowControls="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
