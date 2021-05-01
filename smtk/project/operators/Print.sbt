<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Project "Print" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write" Label="Project - Print" BaseType="operation">
      <BriefDescription>
        Print a project's contents.
      </BriefDescription>

      <AssociationsDef Name="project" NumberOfRequiredValues="1"
                       Extensible="false" OnlyResources="true">
        <Accepts><Resource Name="smtk::project::Project"/></Accepts>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(print)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
