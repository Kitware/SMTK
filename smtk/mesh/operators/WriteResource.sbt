<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the smtk mesh "write resource" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write resource" Label="Mesh - Write Resource" BaseType="operation">
      <AssociationsDef LockType="Read" OnlyResources="true">
          <Accepts><Resource Name="smtk::mesh::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>

        <Void Name="archive" Label="Archive files" Optional="true" IsEnabledByDefault="true" AdvanceLevel="1">
          <BriefDescription>
            Archive all related files into a single archive.
          </BriefDescription>
        </Void>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
