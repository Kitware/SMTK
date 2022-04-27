<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="5" DisplayHint="true">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Group Name="box" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="point 0" NumberOfRequiredValues="3"/>
            <Double Name="point 1" NumberOfRequiredValues="3"/>
            <Double Name="angles" NumberOfRequiredValues="3"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Box Widget Example" Type="Example">
      <Items>
        <Group Name="box" NumberOfGroups="1">
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
              <Double Name="angles">
                <Values>
                  <Val Ith="0">45.0</Val>
                  <Val Ith="1">45.0</Val>
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
        <Att Type="Example" Name="Box Widget Example"
        >
          <ItemViews>
            <View Item="box" Type="Box" Min="point 0" Max="point 1" Angles="angles" ShowControls="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
