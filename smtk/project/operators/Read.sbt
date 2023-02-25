<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Project "Read" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="read" Label="Project - Read" BaseType="operation">
      <BriefDescription>
        Read a project from disk.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Read a project from disk.
        &lt;p&gt;This operator reads the project file and all of its
        resources from disk.
      </DetailedDescription>

      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="SMTK Files (*.smtk)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(read)" BaseType="result">
      <ItemDefinitions>

        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::project::Project"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
