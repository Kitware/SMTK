<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "SetMeshName" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="set mesh name" BaseType="operation" Label="Mesh - Set Name">
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
      <BriefDescription>
        Set a mesh name.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Set a mesh name.
        &lt;p&gt;This operator sets the name of a meshset.
      </DetailedDescription>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(set mesh name)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
