<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="respond to vtk selection" BaseType="operation">
      <BriefDescription>
        Translate VTK block selections into an SMTK component selection.
      </BriefDescription>
      <AssociationsDef
        Name="resource"
        LockType="read"
        NumberOfRequiredValues="1"
        Extensible="yes"
        OnlyResources="true">
        <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions/>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(respond to vtk selection)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
