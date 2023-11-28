<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the task-system "EmplaceWorklet" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="emplace worklet" BaseType="operation">
      <BriefDescription>
        Emplace (instantiate) tasks from a worklet in a task gallery into the task manager.
      </BriefDescription>
      <DetailedDescription>
        This operation constructs tasks, adaptors, and dependencies in the
        task manager by copying a template worklet from the task gallery.
      </DetailedDescription>

      <AssociationsDef Name="project" LockType="Write" NumberOfRequiredValues="1">
        <BriefDescription>The worklet to instantiate into the project.</BriefDescription>
        <Accepts><Resource Name="smtk::project::Project" Filter="smtk::task::Worklet"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <Double Name="location" NumberOfRequiredValues="2">
          <BriefDescription>
            The (x,y) canvas coordinates describing where the center of the worklet's
            tasks should be located.
          </BriefDescription>
          <DefaultValue>0, 0</DefaultValue>
        </Double>
      </ItemDefinitions>

    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(emplace worklet)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
