<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "Merge" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="merge" Label="Model - Merge Entities" BaseType="operation">
      <BriefDescription>
        Merge entities from the same model and of like dimension into a single entity.
      </BriefDescription>
      <AssociationsDef Name="Entities" NumberOfRequiredValues="2" Extensible="true">
        <Accepts><Resource Name="smtk::session::mesh::Resource" Filter="cell"/></Accepts>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(merge)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
