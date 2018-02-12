<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Mesh "Import" Operation -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import" Label="Model - Import" BaseType="operation">
      <BriefDescription>
        Import a model from a mesh file.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Import a model from a mesh file.
        &lt;p&gt;This operator imports a mesh into smtk, and then
        parses its components by dimension and connectivity to form a model.
      </DetailedDescription>
      <ItemDefinitions>

        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Supported Formats (*.h5m *.e *.exo *.ex2 *.g *.gen *.vtu *.vtp *.vtk *.gmv *.ans *.msh *.gmsh *.stl *.2dm *.3dm);;Moab files (*.h5m);;Exodus II Datasets (*.e *.exo *.ex2);;Genesis files (*.g *.gen);;VTK files (*.vtu *.vtp *.vtk);;General Mesh Viewer (*.gmv);;Ansys (*.ans);;Gmsh (*.msh *.gmsh);;STL (*.stl);;AdH 2D Mesh file (*.2dm);;AdH 3D Mesh file (*.3dm);;All files (*.*)">
        </File>

        <Resource Name="resource" Label="Import into" Optional="true" IsEnabledByDefault="false">
          <Accepts>
            <Resource Name="mesh model"/>
          </Accepts>
          <ChildrenDefinitions>
            <String Name="session only" Label="session" Advanced="1">
              <DiscreteInfo DefaultIndex="0">
                <Structure>
                  <Value Enum="this file">import into this file </Value>
                </Structure>
                <Structure>
                  <Value Enum="this session">import into a new file using this file's session</Value>
                </Structure>
              </DiscreteInfo>
            </String>
          </ChildrenDefinitions>
        </Resource>

        <String Name="label" Label="Domain Property" NumberOfRequiredValues="1" AdvanceLevel="1">
	  <DefaultValue></DefaultValue>
	</String>

        <Void Name="construct hierarchy" Label="Construct Model Hierarchy" Optional="true" IsEnabledByDefault="false"/>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>

        <!-- The model imported from the file. -->
        <Resource Name="resource">
          <Accepts>
            <Resource Name="mesh model"/>
          </Accepts>
        </Resource>

        <Component Name="model">
          <Accepts>
            <Resource Name="mesh model" Filter=""/>
          </Accepts>
        </Component>

        <Component Name="mesh_created">
          <Accepts>
            <Resource Name="mesh model" Filter=""/>
          </Accepts>
        </Component>

        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>

      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
