<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB polygon Model "write" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write" Label="Model - Write Resource" BaseType="operation">
      <AssociationsDef LockType="Read" OnlyResources="true">
        <Accepts><Resource Name="smtk::session::polygon::Resource"/></Accepts>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
