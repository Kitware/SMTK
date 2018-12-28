<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="editSource" Label="Model - Edit source" BaseType="operation">
      <BriefDescription>
        Create or edit a source term in the simulation.
      </BriefDescription>
      <AssociationsDef Name="input" Label="Input" NumberOfRequiredValues="1">
        <Accepts>
          <!-- We create a new source if a model is provided or
               edit an existing source if a source is provided. -->
          <Resource Name="smtk::session::oscillator::Resource" Filter="model"/>
          <Resource Name="smtk::session::oscillator::Resource" Filter="aux_geom"/>
        </Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <Group Name="location" NumberOfRequiredValues="1">

          <ItemDefinitions>

            <Double Name="center" Label="center" NumberOfRequiredValues="3">
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
                <Label>Z</Label>
              </ComponentLabels>
              <DefaultValue>0,0,0</DefaultValue>
            </Double>

            <Double Name="radius" Label="radius" NumberOfRequiredValues="1">
              <RangeInfo><Min Inclusive="false">0</Min></RangeInfo>
              <DefaultValue>1</DefaultValue>
            </Double>

          </ItemDefinitions>

        </Group>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(editDomain)" BaseType="result">
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Operation" Title="Source" TopLevel="true">
      <InstancedAttributes>
        <Att Name="editSource" Type="editSource">
          <ItemViews>
            <View Item="location" Type="Sphere" Center="center" Radius="radius"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
