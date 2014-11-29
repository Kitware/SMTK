<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "CreateSphere" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create sphere" BaseType="operator">
      <ItemDefinitions>
        <Double Name="center" NumberOfRequiredValues="3">
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <Double Name="radius" NumberOfRequiredValues="1">
          <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="inner radius" NumberOfRequiredValues="1">
          <DefaultValue>0.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create sphere)" BaseType="result">
      <!-- The sphere created is stored in the base result's "entities" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
