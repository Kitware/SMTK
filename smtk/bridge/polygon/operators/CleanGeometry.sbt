<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the polygon "CleanGeometry" operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="clean geometry" Label="Geometry - Clean" BaseType="operator">
      <BriefDescription>Make geometric entities consistent with modeling assumptions.</BriefDescription>
      <DetailedDescription>
        This operator will split intersecting entities and remove duplicate geometric entities
        so that the input subset of the model is self-consistent.
        After splitting, if two or more edges overlap along their entire length
        (partial overlaps are impossible at this point), then
        all but one of the segments will be deleted (as if they were merged with
        the remaining edge).
        If faces are attached to edges being merged, then an error will be raised.

      </DetailedDescription>
      <AssociationsDef Name="entities" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>cell</MembershipMask>
        <BriefDescription>The cells to clean.</BriefDescription>
        <DetailedDescription>
          Select a set of cells you want to form a self-consistent model after processing.
        </DetailedDescription>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(clean geometry)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
