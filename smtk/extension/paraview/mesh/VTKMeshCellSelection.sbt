<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="vtk mesh cell selection" BaseType="operation">
      <BriefDescription>
        Translate VTK cell-index selections on a mesh into an SMTK meshset selection.
      </BriefDescription>
      <AssociationsDef
        Name="resource"
        LockType="read"
        NumberOfRequiredValues="1"
        Extensible="yes"
        OnlyResources="true">
        <Accepts><Resource Name="smtk::mesh::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions/>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(vtk mesh cell selection)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
