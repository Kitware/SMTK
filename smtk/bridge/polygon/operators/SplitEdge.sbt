<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "SplitEdge" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="split edge" BaseType="operator">
      <BriefDescription>Split a model edge at the given point.</BriefDescription>
      <DetailedDescription>
        Split a model edge in two at the given point.

        The given point must be a non-model-vertex point of the model edge.
        If the model edge has no model vertices, the result will be a single
        new edge with the given point promoted to a model vertex.
        Otherwise 2 new edges are created.
        Regardless, the input edge is always destroyed; it is never modified.
      </DetailedDescription>
      <AssociationsDef Name="edge" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>edge</MembershipMask>
        <BriefDescription>The edge to split.</BriefDescription>
        <DetailedDescription>
          This is a model edge containing at least one point that is not a model-vertex.
          The edge will be removed and replaced by one or two new edges whose endpoint(s)
          are the model vertex.

          When the input edge is a loop with no model vertices,
          then the result is a new edge that has the model vertex as both endpoints;
          otherwise, 2 new model edges will be created.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="point" NumberOfRequiredValues="2" Extensible="yes">
          <BriefDescription>The point where the edge should be split.</BriefDescription>
          <DetailedDescription>
            The world coordinates of the point where the edge should be split.
          </DetailedDescription>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(split edge)" BaseType="result">
      <ItemDefinitions>
        <!-- The edge(s) created are reported in the base result's "created" item. -->
        <!-- The input edge is destroyed and reported in the base result's "expunged" item. -->
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
    <View Type="smtkPolygonEdgeView" Title="Split Polygon Edge">
      <AttributeTypes>
        <Att Type="split edge" />
      </AttributeTypes>
    </View>
  </Views>

</SMTK_AttributeSystem>
