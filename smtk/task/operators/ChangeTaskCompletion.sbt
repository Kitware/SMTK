<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the task-system "ChangeTaskCompletion" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="ChangeTaskCompletion" BaseType="operation">
      <BriefDescription>
        Mark a Task as completed (or un-mark a completed task).
      </BriefDescription>
      <DetailedDescription>
        This operation takes in a task and a completion status.
        It then modifies the task by attempting to set its completion
        status. If no change was made, then the operation will fail.
      </DetailedDescription>

      <AssociationsDef Name="task" LockType="Write" NumberOfRequiredValues="1">
        <BriefDescription>The Task to be renamed.</BriefDescription>
        <Accepts><Resource Name="smtk::project::Project" Filter="smtk::task::Task"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <Void Name="completed" NumberOfRequiredValues="1" IsEnabledByDefault="true" Optional="true">
          <BriefDescription>
            Whether the task should be marked completed (when enabled) or have its completion
            mark removed (when disabled).
          </BriefDescription>
        </Void>
      </ItemDefinitions>

    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(ChangeTaskCompletion)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
