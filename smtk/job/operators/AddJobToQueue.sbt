<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "add job to queue" Operation -->
<SMTK_AttributeResource Version="8" DisplayHint="true">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="add job to queue" Label="Add Job to Queue" BaseType="operation">
      <BriefDescription>
        Job that reports its queue but is not (yet) managed by the queue.
      </BriefDescription>
      <AssociationsDef Name="job" HoldReference="1">
        <Accepts><Resource Name="smtk::job::Queue" Filter="*"/></Accepts>
        <BriefDescription>
          The job to add to the queue.
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(add job to queue)" BaseType="result">
      <!-- ItemDefinitions>
        <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
      </ItemDefinitions -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
