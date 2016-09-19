<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateVertices" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create vertices" Label="Vertex - Create" BaseType="operator">
      <BriefDescription>Create model vertices.</BriefDescription>
      <DetailedDescription>
        Create one or more vertices in the associated model.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The model to which vertices should be added.</BriefDescription>
        <DetailedDescription>
          The model to which vertices should be added.

          This is required in order to project point coordinates into
          the model plane.
        </DetailedDescription>
      </AssociationsDef>

      <ItemDefinitions>
        <Int Name="point dimension" Label="Vertex Information">
          <ChildrenDefinitions>
            <Group Name="2d points" Label="Coordinates"
                   Extensible="true" NumberOfRequiredGroups="1" >
              <ItemDefinitions>
                <Double Name="points" Label="Point" NumberOfRequiredValues="2">
                  <ComponentLabels>
                    <Label>X</Label>
                    <Label>Y</Label>
                  </ComponentLabels>
                </Double>
              </ItemDefinitions>
            </Group>

            <Group Name="3d points" Label="Coordinates"
                   Extensible="true" NumberOfRequiredGroups="1" >
              <ItemDefinitions>
                <Double Name="points" Label="Point" NumberOfRequiredValues="3">
                  <ComponentLabels>
                    <Label>X</Label>
                    <Label>Y</Label>
                    <Label>Z</Label>
                  </ComponentLabels>
                </Double>
              </ItemDefinitions>
            </Group>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="2D: z=0">2</Value>
              <Items>
                <Item>2d points</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="3D">3</Value>
              <Items>
                <Item>3d points</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
          <BriefDescription>Point Geometry used to created Vertices.</BriefDescription>
          <DetailedDescription>
            Depending on the setting of 2D or 3D, this will consist of
            either a set of 2D or 3D points respectively
          </DetailedDescription>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
        <AttDef Type="result(create vertices)" BaseType="result">
      <ItemDefinitions>
        <!-- The vertices created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
