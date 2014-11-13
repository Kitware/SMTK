<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "CreateVertex" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create vertex" BaseType="operator">
      <ItemDefinitions>
        <Double Name="point" NumberOfRequiredValues="3">
          <BriefDescription>The (x,y,z) coordinates of the vertex.</BriefDescription>
        </Double>
        <Int Name="color" NumberOfRequiredValues="1">
          <BriefDescription>The CGM color index assigned to the vertex.</BriefDescription>
          <Min Inclusive="true">0</Min>
          <Max Inclusive="true">15</Max>
          <DefaultValue>1</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create vertex)" BaseType="result">
      <ItemDefinitions>
        <!-- The vertex created. -->
        <ModelEntity Name="vertex" NumberOfRequiredValues="1" MembershipMask="257"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
