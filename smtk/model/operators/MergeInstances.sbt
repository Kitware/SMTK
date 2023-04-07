<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "MergeInstances" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="merge instances" BaseType="operation"
      Label="Model Entities - Merge Instances">
      <AssociationsDef
        Name="instance"
        NumberOfRequiredValues="2"
        Extensible="true"
        HoldReference="true">
        <Accepts><Resource Name="smtk::model::Resource" Filter="instance"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Merge multiple instances with the same prototype into one.
      </BriefDescription>
      <DetailedDescription>
        Merge multiple instances with the same prototype into one.

        An instance is a model entity whose geometry is a transformed
        duplicate of some other model entity; that source entity is
        called its prototype. The result of merging instances will be
        a new instance with a tabular rule (even if all the inputs
        had a different rule for determining placements).
      </DetailedDescription>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(merge instances)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
