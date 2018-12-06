<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "write" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write"
            Label="Attribute - Write Resource" BaseType="operation">
      <AssociationsDef LockType="Read">
        <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
      </AssociationsDef>
    </AttDef>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
