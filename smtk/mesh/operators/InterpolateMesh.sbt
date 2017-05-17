<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "interpolate mesh" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="interpolate mesh"
            Label="Mesh - Interpolate" BaseType="operator">
      <ItemDefinitions>
        <MeshEntity Name="mesh" Label="Mesh" NumberOfRequiredValues="1" Extensible="true" >
          <BriefDescription>The mesh to elevate.</BriefDescription>
        </MeshEntity>

        <String Name="dsname" Label="Field Name" NumberOfRequiredValues="1">
          <BriefDescription>The name of the data set.</BriefDescription>
        </String>

        <Int Name="interpmode" Label="Output Field Type" NumberOfRequiredValues="1" Extensible="false">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Cell Fields">0</Value>
            <Value Enum="Point Fields">1</Value>
          </DiscreteInfo>
        </Int>

        <File Name="ptsfile" Label="Input CSV File" NumberOfValues="1"
              ShouldExist="true" Optional="true" FileFilters="CSV file (*.csv)">
        </File>

        <Group Name="points" Label="Interpolation Points"
               Extensible="true" NumberOfRequiredGroups="0" >
          <BriefDescription>The points and their scalar values to interpolate.</BriefDescription>
          <DetailedDescription>
            An implicit function is described by interpolating points
            and their associated scalar value.
          </DetailedDescription>
          <ItemDefinitions>
            <Double Name="point" Label="Point" NumberOfRequiredValues="4">
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
                <Label>Z</Label>
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
    <AttDef Type="result(interpolate mesh)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0"
                     Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
