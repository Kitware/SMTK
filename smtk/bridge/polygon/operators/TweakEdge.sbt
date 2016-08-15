<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "TweakEdge" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="tweak edge" BaseType="operator">
      <BriefDescription>Tweak the shape of a model edge.</BriefDescription>
      <DetailedDescription>
        Replace all the point coordinates of an edge.

        Self-intersecting edges are not allowed but no tests are performed to prevent them.

        The edge can be split at points along the list of new coordinates by
        passing the indices of those points you wish to promote into model vertices.
      </DetailedDescription>
      <AssociationsDef Name="edge" NumberOfRequiredValues="1" AdvanceLevel="1">
        <MembershipMask>edge</MembershipMask>
        <BriefDescription>An edge to be reshaped.</BriefDescription>
        <DetailedDescription>
          This edge will have its sequence of points replaced with
          those provided in the "points" item.
          If the "promote" item contains any positive integers,
          then the corresponding points will be promoted to model
          vertices by splitting the edge at that point.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="points" NumberOfRequiredValues="4" Extensible="yes" AdvanceLevel="1">
          <BriefDescription>The (x,y,z) coordinates of the edges.</BriefDescription>
          <DetailedDescription>
            The world coordinates of 1 or more edges.

            If only 2 coordinates are specified per point, the third is assumed to be 0.
            Be sure to set the value of the coordinates item as required.
          </DetailedDescription>
        </Double>
        <Int Name="coordinates" NumberOfRequiredValues="1" AdvanceLevel="1">
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
        <Int Name="promote" Optional="true" Extensible="true" AdvanceLevel="1">
          <BriefDescription>Indices into the list of points indicating model vertices.</BriefDescription>
          <DetailedDescription>
            By default, the input edge will have only its tessellation modified.

            If any values between 0 and N - 1 (where N is the number of points provided
            to "tweak edge"), then the edge will be split at the corresponding point
            immediately after the edge is reshaped.

            If the endpoints of the input edge were 1 or 2 model vertices (depending
            on whether the input edge was periodic), then they will be preserved as
            model vertices afterwards, regardless of the value of "promote".
          </DetailedDescription>
        </Int>
        <!-- This is needed for linking with a vtkSMTKOperator that is used as an smtk operator interface
        to vtk pipeline -->
        <Int Name="HelperGlobalID" Label="Unique global ID for a helper object" AdvanceLevel="11" NumberOfRequiredValues="1" Optional="true">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(tweak edge)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0" Extensible="yes"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
     <!-- The customized view "Type" needs to be the the same as in plugin macro
      ADD_SMTK_UI_VIEW(
        OUTIFACES
        OUTSRCS
        CLASS_NAME qtPolygonEdgeOperationView
        VIEW_NAME smtkPolygonEdgeView
        )
      -->
    <View Type="smtkPolygonEdgeView" Title="Tweeak Polygon Edge">
      <AttributeTypes>
        <Att Type="tweak edge" />
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
