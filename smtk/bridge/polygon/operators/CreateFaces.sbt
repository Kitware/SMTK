<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateFaces" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create faces" BaseType="operator">
      <BriefDescription>Create model faces.</BriefDescription>
      <DetailedDescription>
        Create one or more faces in the given model.

        Faces with intersecting edges will cause new (split) edges to be created
        and used in place of those specifying the face.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The model to which faces should be added.</BriefDescription>
        <DetailedDescription>
          The model to which faces should be added.

          This is required in order to project point coordinates into
          the model plane properly and perform intersection tests.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="construction method">
          <ChildrenDefinitions>
            <Double Name="points" NumberOfRequiredValues="6" Extensible="yes">
              <BriefDescription>The (x,y,z) coordinates of the face points.</BriefDescription>
              <DetailedDescription>
                The world coordinates of 3 or more points forming 1 or more faces.
                Multiple faces may be created by specifying "offsets".
                If the default offsets are used, all points are assumed to be
                on the outer loop of a single face.

                If only 2 coordinates are specified per point, the third is assumed to be 0.
                Be sure to set the value of the coordinates item as required.
              </DetailedDescription>
            </Double>
            <Int Name="coordinates" NumberOfRequiredValues="1">
              <BriefDescription>The number of coordinates per vertex.</BriefDescription>
              <DetailedDescription>
                Specify whether 2 or 3 coordinates are provided per vertex.
                When set to 2, the third coordinate is assumed to be 0 for all points.
              </DetailedDescription>
              <DefaultValue>2</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">2</Min>
                <Max Inclusive="true">3</Max>
              </RangeInfo>
            </Int>
            <ModelEntity Name="edges" NumberOfRequiredValues="1" Extensible="true">
              <BriefDescription>The edges defining the boundary of the model face(s).</BriefDescription>
              <DetailedDescription>
                By default, all edges create a single face.
                The first edge passed for each face must be on the outer loop of the face.
                All other loops are considered holes.
                If offsets are specified, then multiple disjoint faces will be created.
              </DetailedDescription>
            </ModelEntity>
            <Int Name="offsets" NumberOfRequiredValues="1" Extensible="true">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>Offsets into the list of points or edges where each face and its holes start.</BriefDescription>
              <DetailedDescription>
                Offsets into the list of points or edges where each face and its holes start.

                The first number specifies an offset into the list of points or edges
                for the first face's outer loop.
                This is followed by the number of holes for the face, followed by an
                integer offset for each hole relative to the start of the face.
                The pattern can be repeated for additional faces.

                An example would be [0, 0,  3, 2, 2, 4,  10, 0].
                This example corresponds to 3 faces.
                The first face has 3 edges in its outer loop and no holes.
                The second face has 2 edges in its outer loop and 2 holes: one hole with 2 edges and another with 3 edges.
                The third face has no holes and uses the remainder of the edges in its outer loop.

                When "points" are specified instead of "edges",
                offset values (except values specifying the number of holes)
                are internally multiplied by the value of "coordinates".
                (Offsets should count the number of points, not coordinates.)
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
            <!-- Option 1: edges and offsets -->
            <Structure>
              <Value Enum="edge ids">0</Value>
              <Items>
                <Item>edges</Item>
                <Item>offsets</Item>
              </Items>
            </Structure>
            <!-- Option 2: all possible faces -->
            <Structure>
              <Value Enum="all non-overlapping faces">0</Value>
              <Items/>
            </Structure>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create faces)" BaseType="result">
      <ItemDefinitions>
        <!-- The faces created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
