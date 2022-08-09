<SMTK_AttributeResource Version="4">
  <Definitions>
    <AttDef Type="elasticity" Label="Elasticity">
      <ItemDefinitions>
        <Double Name="poissons-ratio" Label="Poisson's Ratio (&#x3BD;)">
          <DefaultValue>0.25</DefaultValue>
          <RangeInfo>
            <Min>-1.0</Min>
            <Max>0.5</Max>
          </RangeInfo>
        </Double>
        <Double Name="youngs-modulus" Label="Young's Modulus (&#x395;)">
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="constraint" Label="Constraint" Unique="true">
      <AssociationsDef NumberOfRequiredValues="1" Extensible="true">
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="face" />
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="constraint" Label="Displacement Constraint" Unique="true">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="All Directions">7</Value>
            <Value Enum="X Direction">1</Value>
            <Value Enum="Y Direction">2</Value>
            <Value Enum="Z Direction">4</Value>
            <Value Enum="XY Plane">3</Value>
            <Value Enum="XZ Plane">5</Value>
            <Value Enum="YZ Plane">6</Value>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="load" Label="Load">
      <AssociationsDef NumberOfRequiredValues="1" Extensible="true">
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="face" />
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="load" Label="Force (N)" NumberOfRequiredValues="3">
          <ComponentLabels>
            <Label>X-Force: </Label>
            <Label>Y-Force: </Label>
            <Label>Z-Force: </Label>
          </ComponentLabels>
          <DefaultValue>0.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
    <View Type="Group" Name="Analysis" TopLevel="true" TabPosition="North"
      FilterByAdvanceLevel="false" FilterByCategory="false">
      <Views>
        <View Title="Elasticity" />
        <View Title="Constraints" />
        <View Title="Loads" />
      </Views>
    </View>

    <View Type="Instanced" Title="Elasticity">
      <InstancedAttributes>
        <Att Name="elasticity" Type="elasticity" />
      </InstancedAttributes>
    </View>

    <View Type="Attribute" Title="Constraints">
      <AttributeTypes>
        <Att Type="constraint" />
      </AttributeTypes>
    </View>

    <View Type="Attribute" Title="Loads">
      <AttributeTypes>
        <Att Type="load" />
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
