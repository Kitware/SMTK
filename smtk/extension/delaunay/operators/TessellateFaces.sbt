<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "Tessellate Face" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="tessellate faces" Label="Faces - Tessellate" BaseType="operator">
      <BriefDescription>Tessellate model faces.</BriefDescription>
      <DetailedDescription>
        Tessellate model faces using Delaunay.

        This operation updates the smtk::mesh::Tessellations associated with
        smtk::mesh::Faces using Delaunay. The resulting tessellations are
        composed only of the boundary points.
      </DetailedDescription>
      <AssociationsDef Name="faces" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>face</MembershipMask>
        <BriefDescription>The faces to tessellate.</BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Void Name="validate polygons" Label="Validate Polygons prior to Tessellation"
              Optional="true" AdvanceLevel="1">
          <BriefDescription>Ensure the polygons describing the
          boundaries are valid before tessellating the faces.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(tessellate faces)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="1" Extensible="true"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Operator" Title="Tessellate Face" FilterByAdvanceLevel="true" UseSelectionManager="true">
      <InstancedAttributes>
        <Att Type="tessellate face"/>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeSystem>
