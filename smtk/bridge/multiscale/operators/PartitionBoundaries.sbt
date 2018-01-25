<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="partition boundaries" Label="Model - Partition Boundaries" BaseType="operator">
      <BriefDescription>
        Compute the ratio of the Euler characteristics for a model's
        boundary to its volume.
      </BriefDescription>
      <AssociationsDef Name="Model" NumberOfRequiredValues="1" Extensible="false">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="radius" NumberOfRequiredValues="1">
          <BriefDescription>radius of cooling plate boundary</BriefDescription>
        </Double>
        <Double Name="origin" NumberOfRequiredValues="3">
          <BriefDescription>origin of revolution</BriefDescription>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(partition boundaries)" BaseType="result">
      <ItemDefinitions>
        <!-- The vertex(s) created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
