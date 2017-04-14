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
        smtk::mesh::Face using Delaunay.
      </DetailedDescription>
      <ItemDefinitions>
        <ModelEntity Name="face" NumberOfRequiredValues="1"/>
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
