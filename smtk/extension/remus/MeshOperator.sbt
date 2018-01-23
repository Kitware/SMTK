<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "Mesh" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="mesh" BaseType="operator" AdvanceLevel="11">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1"/>
        <String Name="endpoint" NumberOfRequiredValues="1"/>
        <String Name="remusRequirements" NumberOfRequiredValues="1"/>
        <String Name="meshingControlAttributes" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(mesh)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
