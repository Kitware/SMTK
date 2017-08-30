<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Exodus "EulerCharacteristicRatio" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="euler characteristic ratio" BaseType="operator">
      <BriefDescription>
        Compute the ratio of the Euler characteristics for a model's
        boundary to its volume.
      </BriefDescription>
      <AssociationsDef Name="Model" NumberOfRequiredValues="1" Extensible="false">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(euler characteristic ratio)" BaseType="result">
      <ItemDefinitions>
        <Double Name="value" Label="Euler Characteristic Ratio" NumberOfRequiredValues="1" Extensible="false"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
