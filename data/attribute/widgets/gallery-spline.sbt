<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Group Name="spline" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="points" NumberOfRequiredValues="6" Extensible="true"/>
            <Void Name="closed"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Spline Widget Example" Type="Example">
      <Items>
        <Group Name="spline" NumberOfGroups="1">
          <GroupClusters>
            <Cluster Ith="0">
              <Double Name="points" NumberOfValues="12">
                <Values>
                  <Val  Ith="0">0.0</Val>
                  <Val  Ith="1">0.0</Val>
                  <Val  Ith="2">0.0</Val>

                  <Val  Ith="3">1.0</Val>
                  <Val  Ith="4">0.0</Val>
                  <Val  Ith="5">0.0</Val>

                  <Val  Ith="6">1.0</Val>
                  <Val  Ith="7">1.0</Val>
                  <Val  Ith="8">0.0</Val>

                  <Val  Ith="9">0.0</Val>
                  <Val Ith="10">1.0</Val>
                  <Val Ith="11">0.0</Val>
                </Values>
              </Double>
              <Void Name="closed" IsEnabled="true"/>
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
        <Att Type="Example" Name="Spline Widget Example"
        >
          <ItemViews>
            <View Item="spline" Type="Spline" Points="points" Closed="closed" ShowControls="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
