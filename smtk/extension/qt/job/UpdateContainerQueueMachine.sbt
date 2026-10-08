<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "update container queue machine" Operation -->
<SMTK_AttributeResource Version="8" DisplayHint="true">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="update container queue machine" Label="Update a container queue's machine" BaseType="operation">
      <BriefDescription>
        Job that reports its queue but is not (yet) managed by the queue.
      </BriefDescription>
      <AssociationsDef Name="queue" HoldReference="1" OnlyResources="true">
        <Accepts><Resource Name="smtk::job::Queue"/></Accepts>
        <BriefDescription>
          The queue to update.
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(update container queue machine)" BaseType="result">
      <!-- ItemDefinitions>
        <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
      </ItemDefinitions -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
