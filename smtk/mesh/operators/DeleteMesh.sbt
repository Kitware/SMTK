<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "DeleteMesh" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="delete mesh" BaseType="operation" Label="Mesh - Delete">
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Collection" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Remove a mesh from the model instance.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Remove a mesh from the model instance.
        &lt;p&gt;This operator removes meshes from the application's
        memory. It does not remove meshes from disk.
      </DetailedDescription>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(delete mesh)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
