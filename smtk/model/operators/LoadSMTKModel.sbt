<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "LoadSMTKModel" Operator -->
<SMTK_AttributeSystem Version="3">

  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="load smtk model" Label="Model - Load" BaseType="operator">
      <BriefDecscription>
        Load one or more SMTK model resources from disk
      </BriefDecscription>
      <DetailedDecscription>
        Load models saved in SMTK's native JSON format.
      </DetailedDecscription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" Extensible="true"
          FileFilters="SMTK Model (*.smtk)" Label="SMTK Model File Name " ShouldExist="false">
          <BriefDescription>The filename to load.</BriefDescription>
        </File>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(load smtk model)" BaseType="result">
      <ItemDefinitions>
        <Resource Name="resource" IsEnabledByDefault="true"></Resource>
        <Component Name="model" IsEnabledByDefault="true"></Component>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

</SMTK_AttributeSystem>
