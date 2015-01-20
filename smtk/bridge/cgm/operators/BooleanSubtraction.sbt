<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "BooleanSubtraction" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="subtraction" BaseType="operator">
      <BriefDescription>Subtract tool(s) from the workpiece(s).</BriefDescription>
      <DetailedDescription>
        Subtract the tool body (or bodies) from the workpiece body (or bodies).
        The original workpieces may be modified (the default) or copies made and
        the tool subtracted from these.
        Also, the tool may optionally be imprinted on the workpieces (but is
        not by default).
      </DetailedDescription>
      <AssociationsDef Name="workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The set of workpieces the tool should be removed from.</BriefDescription>
        <DetailedDescription>
          Models from which the tool bodies should be subtracted.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <ModelEntity Name="tools" NumberOfRequiredValues="1" Extensible="true">
          <MembershipMask>model</MembershipMask>
          <BriefDescription>A model to intersect with each of the workpieces in turn.</BriefDescription>
          <DetailedDescription>
            If specificed, the tool is intersected independently with each of the workpieces.
            Otherwise, the workpieces are simultaneously intersected with each other.
            Thus, if a tool is specified, there may be multiple output workpieces which
            only partially overlap.
          </DetailedDescription>
        </ModelEntity>
        <Int Name="imprint workpieces" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <BriefDescription>Should the tool be imprinted onto the workpieces?</BriefDescription>
          <DetailedDescription>
            If true, then the tool is imprinted onto the workpieces.
            Thus any edges on faces of the workpiece that intersect
            the tool(s) will appear on the output bodies.
            The default is false.
          </DetailedDescription>
        </Int>
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
    <AttDef Type="result(subtraction)" BaseType="result">
      <ItemDefinitions>
        <!-- The united body (or bodies) is return in the base result's "entities" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
