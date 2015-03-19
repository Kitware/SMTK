<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Split Face" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="split face" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <ModelEntity Name="face to split" Extensible="1">
        <MembershipMask>face</MembershipMask>
        </ModelEntity>
        <Double Name="feature angle" NumberOfRequiredValues="1">
          <DefaultValue>15.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.</Min>
            <Max Inclusive="true">360.</Max>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(split face)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
