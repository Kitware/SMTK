<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "BooleanUnion" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="translate" BaseType="operator" Associations="model">
      <ItemDefinitions>
        <Double Name="offset" NumberOfRequiredValues="3">
          <DefaultValue>0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(translate)" BaseType="result">
      <ItemDefinitions>
        <!-- The united body (or bodies). -->
        <ModelEntity Name="bodies" NumberOfRequiredValues="1" Extensible="1" MembershipMask="model"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
