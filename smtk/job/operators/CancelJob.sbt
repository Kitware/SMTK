<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "cancel job" Operation -->
<SMTK_AttributeResource Version="8" DisplayHint="true">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="cancel job" Label="Cancel Job" BaseType="operation">
      <BriefDescription>
        Cancel the supplied job.
      </BriefDescription>
      <AssociationsDef Name="job" HoldReference="1">
        <Accepts><Resource Name="smtk::job::Queue" Filter="*"/></Accepts>
        <BriefDescription>
          The job to cancel. It must belong to a queue and be scheduled or running.
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(cancel job)" BaseType="result">
      <!-- ItemDefinitions>
        <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
      </ItemDefinitions -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
