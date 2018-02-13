<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "CreateResource" Operation -->
<SMTK_AttributeSystem Version="3">

  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create resource" Label="Create" BaseType="operation">
      <BriefDecscription>
        Create one or more SMTK resources
      </BriefDecscription>
      <ItemDefinitions>
        <String Name="type" Label="Resource Type" NumberOfRequiredValues="1" Extensible="true">
          <BriefDescription>The unique name of the resource to create.</BriefDescription>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create resource)" BaseType="result">
      <ItemDefinitions>
        <Resource Name="resource" IsEnabledByDefault="true"></Resource>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

</SMTK_AttributeSystem>
