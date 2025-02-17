<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the VTK "Import" Operator -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import" Label="Model - Import" BaseType="operation">
      <!-- Import operations can import a file into an existing
           resource (or an existing resource's session) if one is
           provided. Otherwise, a new resource is created -->
      <AssociationsDef Name="import into" NumberOfRequiredValues="0"
                       Extensible="true" MaxNumberOfValues="1" OnlyResources="true">
        <Accepts><Resource Name="smtk::session::vtk::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <String Name="session only" Label="session" AdvanceLevel="1">
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="this file">import into this file</Value>
            </Structure>
            <Structure>
              <Value Enum="this session">import into a new file using this file's session</Value>
            </Structure>
          </DiscreteInfo>
        </String>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          Extensible="1"
          FileFilters="Exodus II Datasets (*.e *.exo *.ex2);; Genesis files (*.gen);; Label maps (*.vti);; NetCDF files (*.nc *.ncdf);;All files (*.*)">
        </File>
        <String Name="filetype" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false"/>
        <Int Name="readSLACVolumes" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
          <DiscreteInfo DefaultIndex="1">
            <Structure><Value Enum="no">0</Value></Structure>
            <Structure><Value Enum="yes">1</Value></Structure>
          </DiscreteInfo>
        </Int>
        <String Name="label map" NumberOfRequiredValues="1"  Optional="true" >
          <BriefDescription>
            The name of a scalar cell-data array indicating which segment each cell belongs to.
          </BriefDescription>
        </String>
        <ModelEntity Name="preservedUUIDs" NumberOfRequiredValues="0" Extensible="1"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>
        <Component Name="model">
          <Accepts>
            <Resource Name="smtk::session::vtk::Resource" Filter=""/>
          </Accepts>
        </Component>

        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
