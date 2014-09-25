<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Merge" Operator -->
<SMTK_AttributeManager Version="1">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="merge" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1"/>
        <ModelEntity Name="source cell" NumberOfRequiredValues="1"/>
        <ModelEntity Name="target cell" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(merge)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeManager>
