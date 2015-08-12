<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Merge" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="merge face" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
           <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <ModelEntity Name="source cell" NumberOfRequiredValues="0" Extensible="1">
          <MembershipMask>face</MembershipMask>
        </ModelEntity>

        <ModelEntity Name="target cell" NumberOfRequiredValues="1">
          <MembershipMask>face</MembershipMask>
        </ModelEntity>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(merge face)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeSystem>
