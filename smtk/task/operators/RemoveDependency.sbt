<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the task-system "RemoveDependency" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="remove dependency" BaseType="operation">
      <BriefDescription>
        Remove dependencies between pairs of tasks.
      </BriefDescription>
      <DetailedDescription>
        Remove explicit, pre-existing dependencies between pairs of workflow tasks.
        The upstream and downstream tasks will always be marked modified so that
        user interfaces have an opportunity to redraw them with no arc between them

        The upstream task's state will never be changed.
        The downstream task's state may be changed;
        if the downstream task has strict dependency-checking enabled and the
        upstream task is not completed, the downstream task will be updated
        to reflect the removal of the upstream constraint.
        Similarly, if the downstream task has lax dependency-checking enabled
        and the upstream task is unavailable, the downstream task will be
        updated to reflect the removal of the upstream constraint.
      </DetailedDescription>

      <ItemDefinitions>
        <String Name="arc type">
          <BriefDescription>
            The arc type. This value is fixed to be "task dependency;" you may
            not change it as this operation supports only one type of arc removal.
          </BriefDescription>
          <ChildrenDefinitions>
            <Group Name="task endpoints" NumberOfRequiredGroups="0" Extensible="true">
              <ItemDefinitions>

                <Reference Name="from" LockType="Write" NumberOfRequiredValues="1">
                  <Accepts><Resource Name="smtk::project::Project" Filter="smtk::task::Task"/></Accepts>
                  <BriefDescription>
                    The upstream task on which the "to" task depends.
                  </BriefDescription>
                </Reference>

                <Reference Name="to" LockType="Write" NumberOfRequiredValues="1">
                  <Accepts><Resource Name="smtk::project::Project" Filter="smtk::task::Task"/></Accepts>
                  <BriefDescription>
                    The downstream task which is dependent on the "from" task.
                  </BriefDescription>
                </Reference>

              </ItemDefinitions>
            </Group>
          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="task dependency">task dependency</Value>
              <Items>
                <Item>task endpoints</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>

    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(remove dependency)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
