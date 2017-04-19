<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "Triangulate Face" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="triangulate face" Label="Face - Triangulate" BaseType="operator">
      <BriefDescription>Triangulate a model face.</BriefDescription>
      <DetailedDescription>
        Triangulate a model face into a mesh using Delaunay.

        This operation creates an smtk::mesh::MeshSet associated with an
        smtk::mesh::Face using Delaunay. The MeshSet resides in a new
        smtk::mesh::Collection associated with the face's model. The
        resulting triangulation is composed only of the boundary points.
      </DetailedDescription>
      <AssociationsDef Name="face" NumberOfRequiredValues="1">
        <MembershipMask>face</MembershipMask>
        <BriefDescription>The face to triangulate.</BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Void Name="validate polygons" Label="Validate Polygons prior to Triangulation"
              Optional="true" AdvanceLevel="1">
          <BriefDescription>Ensure the polygons describing the
          boundaries are valid before triangulating the face.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(triangulate face)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
