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
      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>model|cell</MembershipMask>
        <BriefDescription>The model to which faces should be added (or edges to collect into a face).</BriefDescription>
        <DetailedDescription>
          The model to which faces should be added or the edges
          that form the boundary of a face (when the construction
          method is "bounding edges").
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
            <Int Name="counts" NumberOfRequiredValues="1" Extensible="true">
              <DefaultValue>-1</DefaultValue>
              <BriefDescription>Offsets into the list of points where each edge starts.</BriefDescription>
              <DetailedDescription>
                The number of points per edge.

                A value of -1 indicates that there is only one edge (which in turn
                must be the outer loop of a simple polygonal face) which is composed
                of all the points listed.

                Note that these integers count the number of points per edge;
                they are not offsets into the list array of point coordinates.
              </DetailedDescription>
            </Int>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="2">
            <!-- Option 0: points, coordinates, and offsets -->
            <Structure>
              <Value Enum="point coordinates">0</Value>
              <Items>
                <Item>points</Item>
                <Item>coordinates</Item>
                <Item>counts</Item>
              </Items>
            </Structure>
            <!-- Option 1: edges and offsets -->
            <Structure>
              <Value Enum="edges">1</Value>
              <Items>
              </Items>
            </Structure>
            <!-- Option 2: all possible faces -->
            <Structure>
              <Value Enum="all non-overlapping faces">2</Value>
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
