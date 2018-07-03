<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CreateFaces" operator -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create faces" Label="Faces - Create All" BaseType="operation">
      <BriefDescription>Create model faces.</BriefDescription>
      <DetailedDescription>
        Create one or more faces in the given model.

        Faces with intersecting edges will cause new (split) edges to be created
        and used in place of those specifying the face.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The model to which faces should be added.</BriefDescription>
        <DetailedDescription>
          The model to which faces should be added.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create faces)" BaseType="result">
      <ItemDefinitions>
        <!-- The faces created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
