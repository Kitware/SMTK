<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "DemoteVertex" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="demote vertex" Label="Vertex - Demote" BaseType="operator">
      <BriefDescription>Demote a model vertex.</BriefDescription>
      <DetailedDescription>
        Demote a model vertex where 1 edge forms a loop with itself or 2 edges meet.

        The vertex will be deleted.

        The given vertex must have 0 or 2 edge-incidences, whether they are from
        the same edge or 2 distinct edges.

        If 2 distinct edges were incident, one of them will be deleted and
        the other will subsume the points along both edge's sequences.
      </DetailedDescription>
      <AssociationsDef Name="vertex" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>vertex</MembershipMask>
        <BriefDescription>The vertex to demote.</BriefDescription>
        <DetailedDescription>
          The vertex will be deleted if the operation is successful (and not otherwise).
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(demote vertex)" BaseType="result">
      <ItemDefinitions>
        <!-- The vertex(s) created are reported in the base result's "created" item. -->
        <!-- The input vertex is destroyed and reported in the base result's "expunged" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
     <!-- The customized view "Type" needs to be the the same as in plugin macro
      ADD_SMTK_UI_VIEW(
        OUTIFACES
        OUTSRCS
        CLASS_NAME qtPolygonVertexOperationView
        VIEW_NAME smtkPolygonVertexView
        )
      -->
    <View Type="smtkPolygonVertexView" Title="Demote Polygon Vertex">
      <AttributeTypes>
        <Att Type="demote vertex" />
      </AttributeTypes>
    </View>
  </Views>

</SMTK_AttributeSystem>
