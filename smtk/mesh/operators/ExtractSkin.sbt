<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "ExtractSkin" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="extract skin" BaseType="operation" Label="Mesh - Extract Skin">
      <BriefDescription>
        Extract a mesh's skin.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Extract a mesh's skin.
        &lt;p&gt;Cells may be created during this process.
      </DetailedDescription>
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(extract skin)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
