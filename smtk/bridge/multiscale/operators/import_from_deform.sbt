<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Multiscale "import_from_deform" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import from deform" Label="Model - Import" BaseType="operation">
      <BriefDescription>
        Import and partition a DEFORM-2D model using Dream3D
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Import and partition a DEFORM-2D model using Dream3D.
        &lt;p&gt;This operator accepts as input a DEFORM-2D point
        tracking file and element file, an attribute from the DEFORM
        simulation, and a list of microscale statistics
        parameters. Using Dream3D, the DEFORM model is imported, cells
        are then clustered into zones according to the input
        attribute, and a microscale profile is generated for each
        zone. The model is partitioned into a number of zones equal to
        the number of microscale statistics provided by the user.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="point-file" Label="DEFORM-2D Point Tracking File" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="DEFORM-2D V11.1 Point Tracking Output Data (*.csv);;All files (*.*)">
          <BriefDescription>DEFORM point tracking file</BriefDescription>
        </File>
        <File Name="element-file" Label="DEFORM-2D Element File" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="DEFORM-2D V11.1 Element File (*.dat);;All files (*.*)">
          <BriefDescription>DEFORM element file</BriefDescription>
        </File>
        <Int Name="timestep" NumberOfRequiredValues="1">
          <BriefDescription>Timestep of the point tracking file to process</BriefDescription>
        </Int>
        <File Name="pipeline-executable" Label="Dream3D Pipeline Executable" NumberOfValues="1" Optional="true" ShouldExist="true">
          <BriefDescription>Dream3D PipelineRunner executable</BriefDescription>
        </File>
	<String Name="attribute" Label="Zoning Attribute">
      <BriefDescription>
        DEFORM attribute field over which the clustering algorithm is performed
      </BriefDescription>
      <DetailedDescription>
        Assuming a relationship between a macroscale property of the
        material and its microscale structure, this property is used
        to partition the model into zones using a k-means clustering algorithm.
      </DetailedDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Effective Strain">Eff. Strain</Value>
            <Value Enum="Effective Strain Rate">Eff. Strain Rate</Value>
            <Value Enum="Relative Density">Relative Density</Value>
            <Value Enum="Damage Factor">Damage Factor</Value>
          </DiscreteInfo>
          <BriefDescription>attribute to use for zoning</BriefDescription>
	</String>
        <Group Name="stats" Label="Microscale Statistics Parameters"
               Extensible="true" NumberOfRequiredGroups="2" >
          <BriefDescription>Microscale statistics feature parameters
          the zoned regions</BriefDescription>
          <DetailedDescription>
            Each zone in the model is associated with a set of
            microscale statistics feature parameters. These parameters
            are used by Dream3D to seed the generation of a
            representative volume element (RVE) associated with the zone.
          </DetailedDescription>
          <ItemDefinitions>
            <Double Name="mu" Label="Mu"/>
            <Double Name="sigma" Label="Sigma"/>
            <Double Name="min_cutoff" Label="Minimum"/>
            <Double Name="max_cutoff" Label="Maximum"/>
          </ItemDefinitions>
        </Group>
        <File Name="output-file" Label="Output Dream3D File" NumberOfRequiredValues="1" ShouldExist="false"
              FileFilters="DREAM3D data file (*.dream3d);;All files (*.*)">
          <BriefDescription>DREAM3D (xdmf) output file</BriefDescription>
	</File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import from deform)" BaseType="result">
      <ItemDefinitions>
        <!-- The model read from the file. -->
        <Component Name="model" NumberOfRequiredValues="1" Extensible="1">
          <Accepts><Resource Name="smtk::bridge::multiscale::Session" Filter="model"/></Accepts>
        </Component>
        <Component Name="mesh_created" NumberOfRequiredValues="1">
          <Accepts><Resource Name="smtk::bridge::multiscale::Session" Filter=""/></Accepts>
        </Component>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
