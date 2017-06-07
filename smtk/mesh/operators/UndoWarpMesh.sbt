<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "undo warp mesh" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="undo warp mesh"
            Label="Mesh - Undo Warp" BaseType="operator">
      <BriefDescription>
        Restore a mesh to its unwarped state.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Restore a mesh to its unwarped state.
        &lt;p&gt;Some operators can distort a mesh's coordinates
        (e.g. Mesh - Elevate). This operator removes the warping
        applied by these operators.
      </DetailedDescription>
      <ItemDefinitions>
        <MeshEntity Name="mesh" Label="Mesh" NumberOfRequiredValues="1" Extensible="true" >
          <BriefDescription>The mesh to restore.</BriefDescription>
        </MeshEntity>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(undo warp mesh)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0"
                     Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
