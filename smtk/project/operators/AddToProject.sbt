<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Project "Create" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="add" Label="Project - Add Resource" BaseType="operation">
      <BriefDescription>
        Add a resource to a project.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Add a resource to a project.
        &lt;p&gt;Add a resource with an optional role to a
        project. The resource must be whitelisted by the project (if
        the project contains a whitelist).
      </DetailedDescription>

      <AssociationsDef Name="project" NumberOfRequiredValues="1"
                       Extensible="false" OnlyResources="true">
        <Accepts><Resource Name="smtk::project::Project"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <Resource Name="resource" Label="Resource">
          <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
        </Resource>

        <String Name="role" Label="Role"/>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(add)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
