<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateVertices" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create vertices" BaseType="operator">
      <ItemDefinitions>
        <Double Name="points" NumberOfRequiredValues="2" Extensible="yes">
          <BriefDescription>The (x,y,z) coordinates of the vertices.</BriefDescription>
          <DetailedDescription>
            The world coordinates of 1 or more vertices.
            If only 2 coordinates are specified, the third is assumed to be 0.

            If more than one vertex's coordinates are given,
            be sure to set the value of the coordinates item as required.
          </DetailedDescription>
        </Double>
        <Int Name="coordinates" NumberOfRequiredValues="1">
          <BriefDescription>The number of coordinates per vertex.</BriefDescription>
          <DetailedDescription>
            When specifying coordinates for more than 1 vertex,
            this dictates how values are passed.
            When set to 2, the third coordinate is assumed to be 0 for all points.
          </DetailedDescription>
          <RangeInfo>
            <Min Inclusive="true">2</Min>
            <Max Inclusive="true">3</Max>
          </RangeInfo>
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
