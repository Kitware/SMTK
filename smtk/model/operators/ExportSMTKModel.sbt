<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ExportModelJSON" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="export smtk model" BaseType="operator">
      <AssociationsDef Name="models" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Export a JSON description of a smtk model.
      </BriefDescription>
      <DetailedDescription>
        Export models in SMTK's native JSON format.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="filename" Label="SMTK Model File Name" NumberOfRequiredValues="1">
          <BriefDescription>The destination file for the JSON.</BriefDescription>
        </File>
        <File Name="nativemodelfilename" Label="Native Model File Name" NumberOfRequiredValues="1">
          <BriefDescription>The destination file for the native model.</BriefDescription>
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(export smtk model)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
