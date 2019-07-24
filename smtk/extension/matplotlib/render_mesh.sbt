<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "render mesh" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="render mesh" Label="Mesh - Render" BaseType="operation">
      <BriefDescription>Render a 2-dimensional mesh using matplotlib.</BriefDescription>
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          FileFilters="Portable Network Graphics (*.png);;Portable Document Format (*.pdf)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(render mesh)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
