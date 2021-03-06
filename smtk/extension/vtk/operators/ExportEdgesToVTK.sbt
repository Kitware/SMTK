<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ExportEdgesToVTK" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="export edges to vtk" BaseType="operation" Label="Model - Export edges to VTK">
      <AssociationsDef Name="models" NumberOfRequiredValues="1">
        <Accepts><Resource Name="smtk::model::Resource" Filter="model"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Export a VTK polydata containing each model edge.
      </BriefDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1">
          <BriefDescription>The destination VTK file.</BriefDescription>
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(export edges to vtk)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
