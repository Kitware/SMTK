<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="revolve" BaseType="operator">
      <AssociationsDef Name="Model(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="sweep-angle" NumberOfRequiredValues="1">
          <BriefDescription>subtended angle in degrees</BriefDescription>
        </Double>
        <Int Name="resolution" NumberOfRequiredValues="1">
          <BriefDescription>resolution of sweep operation</BriefDescription>
        </Int>
        <Double Name="axis-direction" NumberOfRequiredValues="3">
          <BriefDescription>components of three-vector describint the
          rotation axis direction</BriefDescription>
        </Double>
        <Double Name="axis-position" NumberOfRequiredValues="3">
          <BriefDescription>components of three-vector describing the
          rotation axis origin</BriefDescription>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(revolve)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
