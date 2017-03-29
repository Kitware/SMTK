<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ExportModelJSON" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="save smtk model" BaseType="operator">
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
        <File Name="filename" Label="SMTK Model File Name " FileFilters="SMTK Model (*.smtk);;All files (*.*)" NumberOfRequiredValues="1">
          <BriefDescription>The destination file for the JSON.</BriefDescription>
        </File>
        <String Name="rename models" Extensible="0" NumberOfRequiredValues="1" AdvanceLevel="2">
          <BriefDescription>Choose whether and which models will be renamed just before export.</BriefDescription>
          <DetailedDescription>
            By default, models with no names or default names (e.g., "model 0") will be renamed
            just prior to export to include the stem of the filename and, if multiple models were
            saved, a number. If the model name starts with the previous filename (obtained from
            the "url" property attached to the model), then it is considered a default name and
            will be changed if the new filename is different.

            You may opt to force all models to be renamed or to prevent renaming.
          </DetailedDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="only default">only rename models with default names</Value>
            <Value Enum="all">rename all models</Value>
            <Value Enum="none">do not rename models</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(save smtk model)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
