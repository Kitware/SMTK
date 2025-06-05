<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Group Name="disk" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="center" NumberOfRequiredValues="3"/>
            <Double Name="normal" NumberOfRequiredValues="3"/>
            <Double Name="radius"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Disk Widget Example" Type="Example">
      <Items>
        <Group Name="disk" NumberOfGroups="1">
          <GroupClusters>
            <Cluster Ith="0">
              <Double Name="center">
                <Values>
                  <Val Ith="0">0.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">0.0</Val>
                </Values>
              </Double>
              <Double Name="normal">
                <Values>
                  <Val Ith="0">0.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">1.0</Val>
                </Values>
              </Double>
              <Double Name="radius">
                <Values>
                  <Val Ith="0">0.5</Val>
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
        <Att Type="Example" Name="Disk Widget Example"
        >
          <ItemViews>
            <View Item="disk" Type="Disk" Center="center" Normal="normal" Radius="radius" ShowControls="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
