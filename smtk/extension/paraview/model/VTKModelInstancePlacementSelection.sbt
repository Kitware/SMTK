<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="vtk model instance placement selection" BaseType="operation">
      <BriefDescription>
        Translate VTK point or cell-index selections on an instance into an ephemeral instance.
      </BriefDescription>
      <AssociationsDef
        Name="resource"
        LockType="read"
        NumberOfRequiredValues="1"
        Extensible="yes"
        OnlyResources="true">
        <Accepts><Resource Name="smtk::model::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions/>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(vtk model instance placement selection)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
