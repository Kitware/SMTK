<SMTK_AttributeResource Version="7">
  <!-- <Properties>
    <Property Name="smtk.attribute_panel.display_hint" Type="bool">true</Property>
  </Properties> -->

  <Definitions>
    <AttDef Type="Stage1Data" Label="Stage1 Data">
      <ItemDefinitions>
        <Int Name="IntValue" Label="Integer Value">
          <DefaultValue>1</DefaultValue>
        </Int>
        <Double Name="DoubleValue" Label="Double Value">
          <DefaultValue>1.1</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="Stage2Data" Label="Stage2 Data">
      <ItemDefinitions>
        <Int Name="IntValue" Label="Integer Value">
          <DefaultValue>2</DefaultValue>
        </Int>
        <Double Name="DoubleValue" Label="Double Value">
          <DefaultValue>2.2</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Attributes>
    <Att Type="Stage1Data" Name="Stage1Data" />
    <Att Type="Stage2Data" Name="Stage2Data" />
  </Attributes>

  <Views>
    <View Type="Group" Title="Two Operations Example" TopLevel="true" Style="Tiled"
      FilterByAdvanceLevel="false" FilterByCategory="false"
      UseScrollingContainer="false">
      <Views>
        <View Title="Stage1" />
        <View Title="Stage2" />
      </Views>
    </View>

    <View Type="Instanced" Title="Stage1">
      <InstancedAttributes>
        <Att Type="Stage1Data" Name="Stage1Data" />
      </InstancedAttributes>
    </View>

    <View Type="Instanced" Title="Stage2">
      <InstancedAttributes>
        <Att Type="Stage2Data" Name="Stage2Data" />
      </InstancedAttributes>
    </View>

  </Views>
</SMTK_AttributeResource>
