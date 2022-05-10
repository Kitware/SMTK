<SMTK_AttributeResource Version="4">
  <Definitions>
    <AttDef Type="vector-function">
      <ItemDefinitions>
        <Group Name="tabular-data" Extensible="true" NumberOfRequiredGroups="2">
          <ItemDefinitions>
            <Double Name="X" NumberOfRequiredValues="1"></Double>
            <Double Name="Value" NumberOfRequiredValues="3"></Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="boundary-condition">
      <ItemDefinitions>
        <Double Name="velocity" NumberOfRequiredValues="3">
          <ExpressionType>vector-function</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>

   </Definitions>
  <Views>
    <View Type="Attribute" Title="VectorItemTest" TopLevel="true">
      <AttributeTypes>
        <Att Type="boundary-condition">
        </Att>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
