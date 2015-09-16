<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateEdges" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create edges" BaseType="operator">
      <BriefDescription>Create model edges.</BriefDescription>
      <DetailedDescription>
        Create one or more edges in the associated model.

        Self-intersecting edges are not allowed.
        If any edge self-intersects, then new vertices are created intersection points
        and the edge is split at these points.
        In this way, it is possible for specified edges
        to be divided by this operator, resulting in an unexpected number of
        created model edges returned.

        Note that edges are not intersected with other edges (those specified here or
        pre-existing edges in the model).
        Any intersections between different edges are handled when faces are created.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The model to which edges should be added.</BriefDescription>
        <DetailedDescription>
          The model to which edges should be added.

          This is required in order to project point coordinates into
          the model plane properly and perform intersection tests.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="construction method">
          <ChildrenDefinitions>
            <Double Name="points" NumberOfRequiredValues="4" Extensible="yes">
              <BriefDescription>The (x,y,z) coordinates of the edges.</BriefDescription>
              <DetailedDescription>
                The world coordinates of 1 or more edges.

                If only 2 coordinates are specified per point, the third is assumed to be 0.
                Be sure to set the value of the coordinates item as required.
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
            <ModelEntity Name="vertices" NumberOfRequiredValues="2" Extensible="true">
              <BriefDescription>The vertices defining the paths of model edges.</BriefDescription>
              <DetailedDescription>
                By default, all vertices create a single edge.
                However, if offsets are specified, then multiple edges will be created.
              </DetailedDescription>
            </ModelEntity>
            <Int Name="offsets" NumberOfRequiredValues="1" Extensible="true">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>Offsets into the list of points or vertices where each edge starts.</BriefDescription>
              <DetailedDescription>
                Offsets into the list of points or vertices where each edge starts.

                When "points" are specified, each offset value is multiplied by the value of "coordinates".
                Thus, regardless of whether "points" or "vertices" are passed, one would specify
                offsets equal to "[0, 3, 5]" to indicate the first edge has 3 vertices,
                the second edge has 2 vertices, and a third edge exists at the end of these two.
              </DetailedDescription>
            </Int>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <!-- Option 0: points, coordinates, and offsets -->
            <Structure>
              <Value Enum="point coordinates">0</Value>
              <Items>
                <Item>points</Item>
                <Item>coordinates</Item>
                <Item>offsets</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="vertex ids">0</Value>
              <Items>
                <Item>vertices</Item>
                <Item>offsets</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create edges)" BaseType="result">
      <ItemDefinitions>
        <!-- The edges created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
