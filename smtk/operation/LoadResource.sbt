<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "LoadResource" Operation -->
<SMTK_AttributeSystem Version="3">

  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="load resource" Label="Load" BaseType="operation">
      <BriefDecscription>
        Load one or more SMTK resources from disk
      </BriefDecscription>
      <DetailedDecscription>
        Load resources saved in SMTK's native JSON format.
      </DetailedDecscription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" Extensible="true"
          FileFilters="SMTK Resource (*.smtk)" Label="SMTK Resource File Name " ShouldExist="true">
          <BriefDescription>The filename to load.</BriefDescription>
        </File>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(load resource)" BaseType="result">
      <ItemDefinitions>
        <Resource Name="resource" IsEnabledByDefault="true"></Resource>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

</SMTK_AttributeSystem>
