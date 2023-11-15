<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the task-system "RenameTask" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="RenameTask" BaseType="operation">
      <BriefDescription>
        Renames a Task.
      </BriefDescription>
      <DetailedDescription>
        This operation takes in a Task and a new name.  It then
        modifies the Task by setting its name to the new name.
      </DetailedDescription>

      <AssociationsDef Name="task" LockType="Write" NumberOfRequiredValues="1">
        <BriefDescription>The Task to be renamed.</BriefDescription>
        <Accepts><Resource Name="smtk::project::Project" Filter="*"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1">
          <BriefDescription>
            The name that the Task should be set to.
          </BriefDescription>
        </String>
      </ItemDefinitions>

    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(RenameTask)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
