<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "WriteMesh" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="write mesh" Label="Mesh - Save" BaseType="operator">
      <ItemDefinitions>
        <MeshEntity Name="mesh" NumberOfRequiredValues="1" Extensible="true" />
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          FileFilters="MOAB native file (*.h5m *.mhdf);;Exodus II file (*.exo *.exoII *.exo2 *.g *.gen);;Kitware VTK file (*.vtk);;SLAC file (*.slac);;General Mesh Viewer (GMV) file (*.gmv);;Ansys file (*.ans);;Gmsh file (*.msh *.gmsh);;Stereo Lithography file (*.stl)">
        </File>
        <Int Name="write-component" NumberOfRequiredValues="1">
	  <DiscreteInfo DefaultIndex="0">
	    <Value Enum="Entire Collection">0</Value>
            <Value Enum="Only Domain">1</Value>
            <Value Enum="Only Dirichlet">2</Value>
            <Value Enum="Only Neumann">3</Value>
          </DiscreteInfo>
	</Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(write mesh)" BaseType="result">
      <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
