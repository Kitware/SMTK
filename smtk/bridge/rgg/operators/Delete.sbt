<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "Delete" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="delete" BaseType="operator" Label="RGG entities - Delete">
      <BriefDescription>Delete RGG entities.</BriefDescription>
      <DetailedDescription>
        Permanently remove RGG entities (pin, ducts) from a model.
      </DetailedDescription>
      <AssociationsDef Name="entities" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>cell|aux_geom|instance</MembershipMask>
        <BriefDescription>RGG entities to delete.</BriefDescription>
        <DetailedDescription>
          Permanently delete all of these entities.
        </DetailedDescription>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(delete)" BaseType="result">
      <ItemDefinitions>
        <!-- The expunged entities are returned in the base result's "expunged" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
