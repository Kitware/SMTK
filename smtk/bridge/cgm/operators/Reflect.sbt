<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Reflect" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="reflect" BaseType="operator">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model|cell|anydim</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Apply a reflection transform to a set of entities.
      </BriefDescription>
      <DetailedDescription>
        Reflect a set of entities about a plane specified as a base point and direction vector.
      </DetailedDescription>
      <ItemDefinitions>
        <Double Name="direction" NumberOfRequiredValues="3">
          <BriefDescription>The normal to the plane of reflection.</BriefDescription>
          <ComponentLabels>
            <Label>X</Label>
            <Label>Y</Label>
            <Label>Z</Label>
          </ComponentLabels>
        </Double>
        <Double Name="base point" NumberOfRequiredValues="3">
          <BriefDescription>Point about which to reflect.</BriefDescription>
          <ComponentLabels>
            <Label>X</Label>
            <Label>Y</Label>
            <Label>Z</Label>
          </ComponentLabels>
          <DefaultValue>0.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(reflect)" BaseType="result">
      <!-- The translated body (or bodies) are stored in the base result's "entities" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
