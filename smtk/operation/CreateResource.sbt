<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "CreateResource" Operator -->
<SMTK_AttributeSystem Version="3">

  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="create resource" Label="Create" BaseType="operator">
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
