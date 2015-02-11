<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "CreateBody" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create body" BaseType="operator">
      <AssociationsDef Name="free cells" NumberOfRequiredValues="1" Extensible="true">
        <BriefDescription>One or more pre-existing model cells that are not bounding higher-dimensional cells.</BriefDescription>
        <MembershipMask>cell</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="keep inputs" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
        </Int>
        <Int Name="color" NumberOfRequiredValues="1">
          <BriefDescription>The CGM color index assigned to the face.</BriefDescription>
          <Min Inclusive="true">0</Min>
          <Max Inclusive="true">15</Max>
          <DefaultValue>1</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create body)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
