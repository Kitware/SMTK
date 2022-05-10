<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Group Name="sphere" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="point" NumberOfRequiredValues="3"/>
            <Double Name="radius"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Sphere Widget Example" Type="Example">
      <Items>
        <Group Name="sphere" NumberOfGroups="1">
          <GroupClusters>
            <Cluster Ith="0">
              <Double Name="point">
                <Values>
                  <Val Ith="0">0.0</Val>
                  <Val Ith="1">0.0</Val>
                  <Val Ith="2">0.0</Val>
                </Values>
              </Double>
              <Double Name="radius">
                <Values>
                  <Val Ith="0">1.0</Val>
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
        <Att Type="Example" Name="Sphere Widget Example"
        >
          <ItemViews>
            <View Item="sphere" Type="Sphere" Center="point" Radius="radius" ShowControls="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
