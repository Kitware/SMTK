<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Builder" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import" BaseType="operator">
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>
        <!-- The model imported from the file. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
