<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Split Face" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="grow" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <MeshSelection Name="input selection" ModelEntityRef="model">
        </MeshSelection>
        <Double Name="feature angle" NumberOfRequiredValues="1">
          <DefaultValue>30.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.</Min>
            <Max Inclusive="true">180.</Max>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(grow)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="new entities" NumberOfRequiredValues="0" Extensible="1">
          <MembershipMask>face</MembershipMask>
        </ModelEntity>
        <MeshSelection Name="output selection">
        </MeshSelection>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
