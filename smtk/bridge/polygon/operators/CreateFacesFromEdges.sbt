<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateFacesFromEdges" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create faces from edges" Label="Faces - Create from Edges" BaseType="operator">
      <BriefDescription>Create model faces.</BriefDescription>
      <DetailedDescription>
        Create one or more faces in the given model based on a set of edges.

        Faces with intersecting edges will cause new (split) edges to be created
        and used in place of those specifying the face.
      </DetailedDescription>
      <AssociationsDef Name="Model Edges" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>edge</MembershipMask>
        <BriefDescription>The edges used to form the faces.</BriefDescription>
        <DetailedDescription>
          The set of model edges that will be used to create faces from.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create faces from edges)" BaseType="result">
      <ItemDefinitions>
        <!-- The faces created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
