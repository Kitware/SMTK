<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "ExportMesh" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="export mesh" Label="Mesh - Export" BaseType="operator">
      <ItemDefinitions>
        <MeshEntity Name="mesh" NumberOfRequiredValues="1" Extensible="true" />
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          FileFilters="AdH 2D Mesh file (*.2dm);;AdH 3D Mesh file (*.3dm)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(export mesh)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
