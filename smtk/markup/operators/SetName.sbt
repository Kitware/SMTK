<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the markup resource's "create" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="set name" Label="set name" BaseType="operation">
      <AssociationsDef LockType="Write">
        <Accepts>
          <!-- Accept markup resources -->
          <Resource Name="smtk::markup::Resource"/>
          <!-- Accept any markup component -->
          <Resource Name="smtk::markup::Resource" Filter="*"/>
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" Label="name">
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(set name)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
