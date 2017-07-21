<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "EulerCharacteristic" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="euler characteristic" Label="Mesh - Compute Euler Characteristic" BaseType="operator">
      <BriefDescription>
        Compute the Euler characteristic of a mesh.
      </BriefDescription>
      <ItemDefinitions>
        <MeshEntity Name="mesh" NumberOfRequiredValues="1" Extensible="false" />
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(euler characteristic)" BaseType="result">
      <ItemDefinitions>
        <Int Name="value" Label="Euler Characteristic" NumberOfRequiredValues="1" Extensible="false"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
