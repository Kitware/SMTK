<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Mesh "Import" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import" BaseType="operator">
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Moab files (*.h5m);;Exodus II Datasets (*.e *.exo *.ex2);;VTK files (*.vtu *.vtp *.vtk);;All files (*.*)">
        </File>
        <String Name="label" NumberOfRequiredValues="1">
	  <DefaultValue></DefaultValue>
	</String>
        <String Name="filetype" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>
        <!-- The model imported from the file. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
