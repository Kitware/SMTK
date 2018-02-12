<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="revolve" Label="Model - Revolve" BaseType="operation">
      <AssociationsDef Name="Model" NumberOfRequiredValues="1" Extensible="false">
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
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(revolve)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
