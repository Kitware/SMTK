<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "undo elevate mesh" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="undo elevate mesh"
            Label="Mesh - Undo Elevate" BaseType="operation">
      <BriefDescription>
        Restore a mesh to its unelevated state.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Restore a mesh to its unelevated state.
        &lt;p&gt;Some operators can distort a mesh's coordinates
        (e.g. Mesh - Elevate). This operator removes the elevateing
        applied by these operators.
      </DetailedDescription>
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(undo elevate mesh)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
