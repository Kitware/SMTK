<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="fn.initial-condition.temperature" BaseType="" Label="Temperature">
      <ItemDefinitions>
        <Group Name="tabular-data" Label="Tabular Data" Extensible="true" NumberOfRequiredGroups="2">
          <ItemDefinitions>
            <Double Name="X" Label="Indep. Variable" NumberOfRequiredValues="1"></Double>
            <Double Name="Value" Label="Temperature" NumberOfRequiredValues="1"></Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Instanced" Title="Test" TopLevel="true">
      <InstancedAttributes>
        <Att Name="temp" Type="fn.initial-condition.temperature">
          <ItemViews>
            <View Path="/tabular-data" MinNumberOfRows="9" />
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
