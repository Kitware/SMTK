<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "Print" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="print" Label="Model - Print Entity Information" BaseType="operation">
      <BriefDescription>
        Print the underlying data of a mesh session entity.
      </BriefDescription>
      <AssociationsDef Name="Entities" NumberOfRequiredValues="1" Extensible="true">
        <Accepts><Resource Name="smtk::session::mesh::Resource" Filter="cell"/></Accepts>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(print)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
