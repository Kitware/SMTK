<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "PrintMeshInformation" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="print mesh information" BaseType="operation" Label="Mesh - Print Information">
      <AssociationsDef Name="mesh" LockType="Read"
                       NumberOfRequiredValues="1" Extensible="false" HoldReference="true">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Print mesh information to output.
      </BriefDescription>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(print mesh information)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
