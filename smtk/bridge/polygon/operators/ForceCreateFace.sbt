<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "ForceCreateFace" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="force create face" AdvanceLevel="1"  BaseType="operator">
      <BriefDescription>Create a model face without sanity checks.</BriefDescription>
      <DetailedDescription>
        Create a model face from a sequence of points holding an outer loop and
        zero or more inner loops. Each loop will become a single edge.
      </DetailedDescription>

      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>model|cell</MembershipMask>
        <BriefDescription>The model to which faces should be added or an ordered list of edges.</BriefDescription>
        <DetailedDescription>
          When the construction method is set to "points" (index 0), then the association must be a
          polygon-session model to which the face(s) should be added.

          When the construction method is set to "edges" (index 1), then the assocation must
          be an ordered list of edges bounding the face.
          The edges must ordered from head-to-tail in a counterclockwise loop around the
          outside of the face followed by clockwise loops for each hole in the face (if any).
          The orientation of each edge must be provided in the "orientation" item to indicate
          which direction the edge points relative to the loop to which it belongs.
          The edges must all belong to the same polygonal model.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="construction method">
          <ChildrenDefinitions>
            <Double Name="points" NumberOfRequiredValues="6" Extensible="yes">
              <BriefDescription>The (x,y,z) coordinates of the face points.</BriefDescription>
              <DetailedDescription>
                The world coordinates of 3 or more points forming 1 or more faces.
                Inner loops and/or multiple faces may be created by providing a non-default "counts" item.
                If the default counts value (-1) is used, all points are assumed to be
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
            <Int Name="counts" NumberOfRequiredValues="1" Extensible="true">
              <DefaultValue>-1</DefaultValue>
              <BriefDescription>The number of points or edges in each loop of a face.</BriefDescription>
              <DetailedDescription>
                The number of points or edges for the outer and 0 or more inner loops of each face.

                The first number specifies the number of edges in the outer loop of the face.
                The second number specifies the number of inner loops and is followed by
                the number of edges in each of those inner loops, respectively.
                This pattern may be repeated for multiple faces.

                If a single value of -1 is provided, then all points/edges are assumed to lie
                on the outer loop of a single face.
              </DetailedDescription>
            </Int>
            <Int Name="orientations" NumberOfRequiredValues="1" Extensible="true">
              <DefaultValue>1</DefaultValue>
              <BriefDescription>The orientation of each edge (+1 for positive, -1 for negative).</BriefDescription>
              <DetailedDescription>
                The orientation of each edge (+1 for positive, -1 for negative) to use for the face.
                Outer loops must be counter-clockwise and inner loops must be clockwise with respect to
                the face normal (the positive z axis for default models).

                This item must have the same number of values as the list of associated edges.
              </DetailedDescription>
            </Int>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="1">
            <!-- Option 0: points, coordinates, and counts -->
            <Structure>
              <Value Enum="points">0</Value>
              <Items>
                <Item>points</Item>
                <Item>coordinates</Item>
                <Item>counts</Item>
              </Items>
            </Structure>
            <!-- Option 1: oriented edges and counts -->
            <Structure>
              <Value Enum="edges">1</Value>
              <Items>
                <Item>orientations</Item>
                <Item>counts</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(force create face)" BaseType="result">
      <ItemDefinitions>
        <!-- The faces created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
