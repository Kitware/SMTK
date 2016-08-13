<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateVertices" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create edge from points" Label="Edge - Create from Points" BaseType="operator">
      <BriefDescription>Create model edge based on a list of points.</BriefDescription>
      <DetailedDescription>
        Create one or more vertices in the associated model.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The model to which edge should be added.</BriefDescription>
        <DetailedDescription>
          The model to which the edge should be added.

          This is required in order to project point coordinates into
          the model plane.
        </DetailedDescription>
      </AssociationsDef>

      <ItemDefinitions>
        <Int Name="pointGeometry" Label="Vertex Information">
          <ChildrenDefinitions>
            <Group Name="2DPoints" Label="Coordinates"
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

            <Group Name="3DPoints" Label="Coordinates"
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
                <Item>2DPoints</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="3D">3</Value>
              <Items>
                <Item>3DPoints</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
          <BriefDescription>Point Geometry used to created the Edge.</BriefDescription>
          <DetailedDescription>
            Depending on the setting of 2D or 3D, this will consist of
            either a set of 2D or 3D points respectively
          </DetailedDescription>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
        <AttDef Type="result(create edge from points)" BaseType="result">
      <ItemDefinitions>
        <!-- The edge created is reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
