<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateEdge" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create edge" BaseType="operator">
      <BriefDescription>Create model edge(s).</BriefDescription>
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
      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>model|cell</MembershipMask>
        <BriefDescription>Vertices to join into an edge or the model to which edges should be added.</BriefDescription>
        <DetailedDescription>
          You must either (a) associate 2 or more model vertices to this
          operator or (b) associate a model into which edges should be
          inserted and specify points to connect into edges.

          You must not specify both a model and a list of vertices.
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
                <!-- vertices are associated with the operator in this case -->
                <Item>offsets</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create edge)" BaseType="result">
      <ItemDefinitions>
        <!-- The edges created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
