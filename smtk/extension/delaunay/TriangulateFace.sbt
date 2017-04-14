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
        smtk::mesh::Face using Delaunay. The MeshSet resides in the
        smtk::mesh::Collection with the same UUID as the Face's model. If this
        collection does not yet exist during the construction of the mesh, it is
        created and populated with the MeshSet.
      </DetailedDescription>
      <ItemDefinitions>
        <ModelEntity Name="face" NumberOfRequiredValues="1"/>
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
