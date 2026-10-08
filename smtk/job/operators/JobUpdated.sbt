<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "job updated" Operation -->
<SMTK_AttributeResource Version="8" DisplayHint="true">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="job updated" Label="Update a job with progress" BaseType="operation">
      <BriefDescription>
        This operation is launched by queues when the job stage/state/status should be modified.
      </BriefDescription>
      <AssociationsDef Name="job" HoldReference="1">
        <Accepts><Resource Name="smtk::job::Queue" Filter="*"/></Accepts>
        <BriefDescription>
          The job to update.
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="state">
          <BriefDescription>The new state for the associated job.</BriefDescription>
          <DiscreteInfo DefaultIndex="0" Optional="true">
            <Value Enum="Unscheduled">0</Value>
            <Value Enum="Scheduled">1</Value>
            <Value Enum="Running">2</Value>
            <Value Enum="Canceled">3</Value>
            <Value Enum="Completed">4</Value>
          </DiscreteInfo>
        </Int>
        <Int Name="status" Optional="true">
          <BriefDescription>The new status for the associated job.</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Pending">0</Value>
            <Value Enum="Succeeded">1</Value>
            <Value Enum="Failed">2</Value>
            <Value Enum="Terminated">3</Value>
          </DiscreteInfo>
        </Int>
        <Int Name="stage" Optional="true">
          <BriefDescription>The stage of the job has changed.</BriefDescription>
        </Int>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(job updated)" BaseType="result">
      <!-- ItemDefinitions>
        <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
      </ItemDefinitions -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
