<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "edge" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="split edge" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <MeshSelection Name="selection" ModelEntityRef="model">
          <MembershipMask>edge|vertex</MembershipMask>
        </MeshSelection>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(split edge)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
