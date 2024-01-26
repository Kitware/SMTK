<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Project "Create" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create" Label="Project" BaseType="operation">
      <BriefDescription>
        Create a project instance for a specified type.
      </BriefDescription>
      <DetailedDescription>
        Create a project instance. The project type must be one that
        has been registered with the project manager.
      </DetailedDescription>

      <ItemDefinitions>

        <String Name="typeName" Label="Project Type">
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create)" BaseType="result">
      <ItemDefinitions>

        <Resource Name="resource" HoldReference="true" Extensible="true" NumberOfRequiredValues="0">
          <Accepts>
            <Resource Name="smtk::project::Project"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
