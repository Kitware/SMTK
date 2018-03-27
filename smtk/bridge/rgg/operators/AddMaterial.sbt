<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "AddMaterial" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="add material" Label="Model - Add Material" BaseType="operator">
      <BriefDescription>
        Add a user defined material.
        Warning: For now it would clear all predefined materials in smtk.
      </BriefDescription>
      <DetailedDescription>
        Add a user defined material.
        Warning: For now it would clear all predefined materials in smtk.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="0">
        </String>
        <String Name="label" NumberOfRequiredValues="1" AdvanceLevel="0">
        </String>
        <Double Name="color" NumberOfRequiredValues="4" AdvanceLevel="0">
          <BriefDescription>
            r, g, b, a. Each one should stay within [0,1] range.
          </BriefDescription>
          <DetailedDescription>
            r, g, b, a. Each one should stay within [0,1] range.
          </DetailedDescription>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(add material)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
