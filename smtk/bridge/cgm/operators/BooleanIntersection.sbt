<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "BooleanIntersection" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="intersection" BaseType="operator">
      <BriefDescription>Intersect the workpiece volumes.</BriefDescription>
      <DetailedDescription>
        Intersect the workpieces, either with each other or with a tool if one is specified.
      </DetailedDescription>
      <AssociationsDef Name="workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>A set of workpieces to intersect with each other (or the tool, if one is given).</BriefDescription>
        <DetailedDescription>
          Models that should be intersected with each other.
          If a tool is specified, each workpiece model is intersected independently
          with the tool.
          Otherwise, the workpieces are simultaneously intersected with each other.
          If no tool is specified, at least 2 workpieces must be given.
          Otherwise, 1 or more workpieces is acceptable.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <ModelEntity Name="tool" NumberOfRequiredValues="0" MaximumNumberOfValues="1" Extensible="true" Optional="true">
          <MembershipMask>model</MembershipMask>
          <BriefDescription>A model to intersect with each of the workpieces in turn.</BriefDescription>
          <DetailedDescription>
            If specificed, the tool is intersected independently with each of the workpieces.
            Otherwise, the workpieces are simultaneously intersected with each other.
            Thus, if a tool is specified, there may be multiple output workpieces which
            only partially overlap.
          </DetailedDescription>
        </ModelEntity>
        <Int Name="keep inputs" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <BriefDescription>Should the inputs be copied before intersection?</BriefDescription>
          <DetailedDescription>
            If true, then copies of the workpieces are intersected and the
            input models are untouched while their intersections are added
            as new models in the session.
            Otherwise, the workpieces are modified and the tool consumed
            by the operation (this is the default).
          </DetailedDescription>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(intersection)" BaseType="result">
      <ItemDefinitions>
        <!-- The united body (or bodies) is return in the base result's "modified" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
