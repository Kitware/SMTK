<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "Tessellate Face" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="tessellate face" Label="Face - Tessellate" BaseType="operator">
      <BriefDescription>Tessellate a model face.</BriefDescription>
      <DetailedDescription>
        Tessellate a model face using Delaunay.

        This operation updates the smtk::mesh::Tessellation associated with an
        smtk::mesh::Face using Delaunay. The resulting tessellation is
        composed only of the boundary points.
      </DetailedDescription>
      <AssociationsDef Name="face" NumberOfRequiredValues="1">
        <MembershipMask>face</MembershipMask>
        <BriefDescription>The face to tessellate.</BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Void Name="validate polygons" Label="Validate Polygons prior to Tessellation"
              Optional="true" AdvanceLevel="1">
          <BriefDescription>Ensure the polygons describing the
          boundaries are valid before tessellating the face.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(tessellate face)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
