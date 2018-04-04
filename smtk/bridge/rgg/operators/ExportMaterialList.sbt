<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "ExportMaterialList" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="export material list" Label="Model - Export Material List" BaseType="operator">
      <BriefDescription>Export the material list for RGG model.</BriefDescription>
      <DetailedDescription>
        Export the available materials for an RGG model to a SON file.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
              FileFilters="SON files (*.son)"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="result(export material list)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
