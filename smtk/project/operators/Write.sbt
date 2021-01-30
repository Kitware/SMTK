<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Project "Write" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write" Label="Project - Write" BaseType="operation">
      <BriefDescription>
        Write a project to disk.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Write a project to disk.
        &lt;p&gt;This operator creates a file representing the
        selected project and saves it and its contents to disk.
      </DetailedDescription>

      <AssociationsDef Name="project" LockType="Read" NumberOfRequiredValues="1"
                       Extensible="false" OnlyResources="true">
        <Accepts><Resource Name="smtk::project::Project"/></Accepts>
      </AssociationsDef>

    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
