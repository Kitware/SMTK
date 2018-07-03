<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ExportModelJSON" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="export model json" Label="Model - Export JSON" BaseType="operation" AdvanceLevel="10">
      <AssociationsDef Name="models" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Export a JSON description of a model.
      </BriefDescription>
      <DetailedDescription>
        Export models in SMTK's native JSON format.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1">
          <BriefDescription>The destination file for the JSON.</BriefDescription>
        </File>
        <Int Name="flags" NumberOfRequiredValues="1">
          <BriefDescription>Flags used to control what is written.</BriefDescription>
          <DefaultValue>255</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(export model json)" BaseType="result">
      <!-- The modified entities are stored in the base result's "modified" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
