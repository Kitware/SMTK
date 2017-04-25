<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "interpolate mesh elevation" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="interpolate mesh elevation" Label="Mesh - Interpolate Elevation" BaseType="operator">
      <ItemDefinitions>
        <MeshEntity Name="mesh" NumberOfRequiredValues="1" Extensible="true" >
          <BriefDescription>The mesh to elevate.</BriefDescription>
        </MeshEntity>

        <Group Name="points" Label="Interpolation Points"
               Extensible="true" NumberOfRequiredGroups="1" >
          <BriefDescription>The points and their scalar values to interpolate.</BriefDescription>
          <DetailedDescription>
            An implicit function is described by interpolating points
            and their associated scalar value.
          </DetailedDescription>
          <ItemDefinitions>
            <Double Name="point" Label="Point" NumberOfRequiredValues="3">
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
                <Label>Scalar</Label>
              </ComponentLabels>
            </Double>
          </ItemDefinitions>
        </Group>

        <Double Name="power" NumberOfRequiredValues="1" Extensible="no">
          <BriefDescription>The weighting power used to interpolate
          source points.</BriefDescription>
          <DetailedDescription>
            The weighting power used to interpolate source points.

            The operator uses Shepard's method to interpolate between
            points. The inverse distance value is exponentiated by
            this unitless value when computing weighting coefficients.
          </DetailedDescription>
          <DefaultValue>1.</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(interpolate mesh elevation)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0"
                     Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
