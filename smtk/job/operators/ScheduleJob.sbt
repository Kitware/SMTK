<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "schedule job" Operation -->
<SMTK_AttributeResource Version="8" DisplayHint="true">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="schedule job" Label="Schedule Job" BaseType="operation">
      <BriefDescription>
        Job that reports its queue but is not (yet) managed by the queue.
      </BriefDescription>
      <AssociationsDef Name="job" HoldReference="1">
        <Accepts><Resource Name="smtk::job::Queue" Filter="*"/></Accepts>
        <BriefDescription>
          The job to schedule.
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(schedule job)" BaseType="result">
      <!-- ItemDefinitions>
        <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
      </ItemDefinitions -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
