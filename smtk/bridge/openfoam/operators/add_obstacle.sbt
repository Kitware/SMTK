<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the OpenFOAM "add_obstacle" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="add obstacle" Label="Model - Add Obstacle" BaseType="operator">
      <BriefDescription>
        Create a wind tunnel for OpenFOAM
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Create an obstacle to an OpenFOAM wind tunnel model.
        &lt;p&gt;This operator accepts as input an OpenFOAM wind
        tunnel and either a Wavefront (.obj) or a stereolithography
        (.stl) auxiliary geometry representing a triangulated surface
        geometry. It uses OpenFOAM's snappyHexMesh to combine the wind
        tunnel background mesh and the surface geometry.

      </DetailedDescription>
      <ItemDefinitions>

        <ModelEntity Name="wind tunnel" Label = "Wind Tunnel" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
          <BriefDescription>
            A wind tunnel model to be used as the background mesh.
          </BriefDescription>
        </ModelEntity>

        <ModelEntity Name="obstacle" Label = "Triangulated
                                                        Surface Auxiliary Geometry" NumberOfRequiredValues="1">
          <MembershipMask>aux_geom</MembershipMask>
          <BriefDescription>
            An external triangulated surface geometry to be placed in
            the wind tunnel.
          </BriefDescription>
        </ModelEntity>

        <Double Name="x dimensions" Label="Refinement Box X Dimensions" NumberOfRequiredValues="2">
          <ComponentLabels>
            <Label>min</Label>
            <Label>max</Label>
          </ComponentLabels>
          <BriefDescription>X coordinate values for refinement box</BriefDescription>
          <DefaultValue>-1,8</DefaultValue>
        </Double>

        <Double Name="y dimensions" Label="Refinement Box Y Dimensions" NumberOfRequiredValues="2">
          <ComponentLabels>
            <Label>min</Label>
            <Label>max</Label>
          </ComponentLabels>
          <BriefDescription>Y coordinate values for refinement box</BriefDescription>
          <DefaultValue>-.7,.7</DefaultValue>
        </Double>

        <Double Name="z dimensions" Label="Refinement Box Z Dimensions" NumberOfRequiredValues="2">
          <ComponentLabels>
            <Label>min</Label>
            <Label>max</Label>
          </ComponentLabels>
          <BriefDescription>Z coordinate values for refinement box</BriefDescription>
          <DefaultValue>0,2.5</DefaultValue>
        </Double>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(add obstacle)" BaseType="result">
      <ItemDefinitions>
        <!-- The created wind tunnel. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
