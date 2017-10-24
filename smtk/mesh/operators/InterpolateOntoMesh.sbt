<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "interpolate onto mesh" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="interpolate onto mesh"
            Label="Mesh - Interpolate data onto mesh" BaseType="operator">
      <BriefDescription>
        Create a field on mesh nodes/elements from interpolated 3-dimensional data.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Create a field on mesh nodes/elements from interpolated 3-dimensional data.
        &lt;p&gt;This operator accepts as input three-dimensional points
        with associated scalar values; it interpolates these values
        onto either the points or the cells of the mesh using
        Shepard's method for interpolation. The input points can be
        inserted manually or read from a CSV file.
      </DetailedDescription>
      <ItemDefinitions>
        <MeshEntity Name="mesh" Label="Mesh" NumberOfRequiredValues="1" Extensible="true" >
          <BriefDescription>The mesh onto which the data is interpolated.</BriefDescription>
        </MeshEntity>

        <String Name="dsname" Label="Field Name" NumberOfRequiredValues="1">
          <BriefDescription>The name of the generated data set.</BriefDescription>
        </String>

        <Int Name="interpmode" Label="Output Field Type" NumberOfRequiredValues="1" Extensible="false">
          <BriefDescription>Interpolate the data as cell-centered or point-centered on the mesh.</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Cell Fields">0</Value>
            <Value Enum="Point Fields">1</Value>
          </DiscreteInfo>
        </Int>

        <File Name="ptsfile" Label="Input CSV File" NumberOfValues="1"
              ShouldExist="true" Optional="true" FileFilters="CSV file (*.csv)">
          <BriefDescription>Input file containing rows of 4 comma separated values: x, y, z, value.</BriefDescription>
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
    <AttDef Type="result(interpolate onto mesh)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0"
                     Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
