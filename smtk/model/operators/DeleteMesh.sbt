<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "DeleteMesh" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="delete mesh" BaseType="operator" Label="Delete - Mesh">
      <ItemDefinitions>
        <MeshEntity Name="mesh" NumberOfRequiredValues="1" Extensible="true" />
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <AttDef Type="result(delete mesh)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_expunged" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
