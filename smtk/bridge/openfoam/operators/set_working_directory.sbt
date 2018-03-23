<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the OpenFOAM "set_working_directory" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="set working directory" Label="Session - Set Working Directory" BaseType="operator">
      <BriefDescription>
        Set working directory for creating/manipulating OpenFOAM models
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Set working directory for creating/manipulating OpenFOAM models.
        &lt;p&gt;OpenFOAM's workflow operates around a top-level
        working directory. This operator sets that directory for
        subsequent OpenFOAM actions.
      </DetailedDescription>
      <ItemDefinitions>

        <Directory Name="working directory" Label="Working Directory" NumberOfRequiredValues="1">
          <BriefDescription>The directory in which the active OpenFOAM
          workflow resides.</BriefDescription>
        </Directory>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(set working directory)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
