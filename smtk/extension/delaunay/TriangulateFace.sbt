<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "Triangulate Face" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="triangulate face" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="face" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(triangulate face)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
