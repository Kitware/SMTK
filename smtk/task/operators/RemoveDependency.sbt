<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the task-system "AddDependency" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="add dependency" BaseType="operation">
      <BriefDescription>
        Add a task as a dependency to another.
      </BriefDescription>
      <DetailedDescription>
        This operation constructs tasks, adaptors, and dependencies in the
        task manager by copying a template worklet from the task gallery.
      </DetailedDescription>

      <AssociationsDef Name="from" NumberOfRequiredValues="1">
        <BriefDescription>The task to serve as the dependency.</BriefDescription>
        <Accepts><Resource Name="smtk::project::Project" Filter="smtk::task::Task"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <Component Name="to" LockType="Write" NumberOfRequiredValues="1">
          <Accepts><Resource Name="smtk::project::Project" Filter="smtk::task::Task"/></Accepts>
          <BriefDescription>
            The task which should be dependent on the associated task.
          </BriefDescription>
        </Component>
      </ItemDefinitions>

    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(add dependency)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
