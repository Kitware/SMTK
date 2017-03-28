<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="partition boundaries" BaseType="operator">
      <AssociationsDef Name="Model(s)" NumberOfRequiredValues="1" Extensible="true">
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
    <AttDef Type="result(partition boundaries)" BaseType="result">
      <ItemDefinitions>
        <!-- The vertex(s) created are reported in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
