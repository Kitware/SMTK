<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateEdgeFromVertices" operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create edge from vertices" Label="Edge - Create from Vertices" BaseType="operation">
      <BriefDescription>Create model edge.</BriefDescription>
      <DetailedDescription>
        Create a model edge from a pair of model vertices.
      </DetailedDescription>
      <AssociationsDef Name="Model Vertices" NumberOfRequiredValues="2" Extensible="no">
        <MembershipMask>vertex</MembershipMask>
        <BriefDescription>The vertices used to form the edge.</BriefDescription>
        <DetailedDescription>
          The pair of model vertices to be used to create a model edge..
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create edge from vertices)" BaseType="result">
      <ItemDefinitions>
        <!-- The edge created is reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
