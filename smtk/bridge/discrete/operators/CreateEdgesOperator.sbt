<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Create Edges" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create edges" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create edges)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
