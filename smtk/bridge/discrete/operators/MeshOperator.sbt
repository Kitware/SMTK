<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Mesh" Operator -->
<SMTK_AttributeManager Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="mesh" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1"/>
        <String Name="endpoint" NumberOfRequiredValues="1"/>
        <String Name="remusRequirements" NumberOfRequiredValues="1"/>
        <String Name="meshingControlAttributes" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(mesh)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeManager>
