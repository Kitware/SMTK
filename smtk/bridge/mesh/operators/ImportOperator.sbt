<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Mesh "Import" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import" Label="Model - Import from Mesh" BaseType="operator">
      <BriefDescription>
        Import a model from a mesh.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Import a model from a mesh.
        &lt;p&gt;This operator imports a mesh into smtk, and then
        parses its components by dimension and connectivity to form a model.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Supported Formats (*.h5m *.e *.exo *.ex2 *.g *.gen *.vtu *.vtp *.vtk *.gmv *.ans *.msh *.gmsh *.stl *.2dm *.3dm);;Moab files (*.h5m);;Exodus II Datasets (*.e *.exo *.ex2);;Genesis files (*.g *.gen);;VTK files (*.vtu *.vtp *.vtk);;General Mesh Viewer (*.gmv);;Ansys (*.ans);;Gmsh (*.msh *.gmsh);;STL (*.stl);;AdH 2D Mesh file (*.2dm);;AdH 3D Mesh file (*.3dm);;All files (*.*)">
        </File>
        <String Name="label" Label="Domain Property" NumberOfRequiredValues="1" AdvanceLevel="1">
	  <DefaultValue></DefaultValue>
	</String>
        <Void Name="construct hierarchy" Label="Construct Model Hierarchy" Optional="true" IsEnabledByDefault="true"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>
        <!-- The model imported from the file. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
