<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "edge" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="set mesh property" BaseType="operator" AdvanceLevel="11">
      <AssociationsDef Name="model" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Set (or remove) a property value on a set of meshsets.
      </BriefDescription>
      <DetailedDescription>
        Set (or remove) a property value on a set of meshsets.
        The string, integer, and floating-point values are all optional.
        Any combination may be specified.
        All that are specified are set; those unspecified are removed.

        For example, specifying both a string and an integer value for
        the "foo" property would set those values in the mesh collection's
        string and integer maps while removing "foo" from the associated
        meshsets' floating-point map.
      </DetailedDescription>

      <ItemDefinitions>
        <MeshEntity Name="meshes" NumberOfRequiredValues="1" Extensible="true">
        </MeshEntity>

        <String Name="name" NumberOfRequiredValues="1">
          <BriefDescription>The name of the property to set.</BriefDescription>
        </String>
        <Double Name="float value" NumberOfRequiredValues="0" Extensible="true">
          <BriefDescription>Floating-point value(s) of the property.</BriefDescription>
        </Double>
        <String Name="string value" NumberOfRequiredValues="0" Extensible="true">
          <BriefDescription>String value(s) of the property.</BriefDescription>
        </String>
        <Int Name="integer value" NumberOfRequiredValues="0" Extensible="true">
          <BriefDescription>Integer value(s) of the property.</BriefDescription>
        </Int>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(set mesh property)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
