<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Project "Create" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create" Label="Project - Create" BaseType="operation">
      <BriefDescription>
        Create a basic project type.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Create a basic project type.
        &lt;p&gt;Options are available to white-list associated resources and operations.
      </DetailedDescription>

      <ItemDefinitions>

        <String Name="typeName" Label="Project Type">
          <!-- [defined programmatically] -->
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create)" BaseType="result">
      <ItemDefinitions>

        <Resource Name="project" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::project::Project"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeResource>
