<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Copy" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="copy" BaseType="operator">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model|cell|anydim</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Copy the entity.
      </BriefDescription>
      <DetailedDescription>
        Make a copy of the given entity.

        Note that CGM treats bodies differently than other entities.
      </DetailedDescription>
      <ItemDefinitions>
        <!-- No parameters. Either you want a copy or you don't. -->
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(copy)" BaseType="result">
      <!-- The translated entities are stored in the base result's "created" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
