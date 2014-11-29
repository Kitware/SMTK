<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "BooleanUnion" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="rotate" BaseType="operator">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="axis" NumberOfRequiredValues="3">
        </Double>
        <Double Name="center" NumberOfRequiredValues="3">
          <DefaultValue>0</DefaultValue>
        </Double>
        <Double Name="angle" NumberOfRequiredValues="1">
          <!-- Rotation angle, in degrees, about the line
               defined by the center point and (non-zero) axis
            -->
          <DefaultValue>45</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(rotate)" BaseType="result">
      <!-- The rotated body (or bodies) are stored in the base result's "entities" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
