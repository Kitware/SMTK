<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "CreateCylinder" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create cylinder" BaseType="operator">
      <ItemDefinitions>
        <Double Name="height" NumberOfRequiredValues="1">
          <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="major base radius" NumberOfRequiredValues="1">
          <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="minor base radius" NumberOfRequiredValues="1">
          <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="major top radius" NumberOfRequiredValues="1">
          <Min Inclusive="true">0</Min>
          <DefaultValue>0.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create cylinder)" BaseType="result">
      <!-- The cylinder created is stored in the base result's "created" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
