<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Project "Remove" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="remove" Label="Project - Remove Resource" BaseType="operation">
      <BriefDescription>
        Remove a resource from a project.
      </BriefDescription>

      <AssociationsDef Name="project" NumberOfRequiredValues="1"
                       Extensible="false" OnlyResources="true">
        <Accepts><Resource Name="smtk::project::Project"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <Resource Name="resource" Label="Resource">
          <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
          <Rejects><Resource Name="smtk::project::Project"/></Rejects>
        </Resource>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(remove)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
