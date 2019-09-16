<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "MergeCoincidentPoints" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="merge coincident points" BaseType="operation"
            Label="Mesh - Merge Coincident Points">
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="tolerance" Label="Tolerance" NumberOfRequiredValues="1">
          <BriefDescription>
            The maximum distance between two points concidered to be coincident.
          </BriefDescription>
          <DefaultValue>1.e-6</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0.</Min>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
      <BriefDescription>
        Merge coincident points.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Merge coincident points.
        &lt;p&gt;This operator merges points that are less than a
        user-defined tolerance from each other.
      </DetailedDescription>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(merge coincident points)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
