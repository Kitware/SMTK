<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="Operation.xml"/>
    <AttDef Type="dummy" BaseType="operation">
      <AssociationsDef LockType="Write" HoldReference="true" OnlyResources="true">
        <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
      </AssociationsDef>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
