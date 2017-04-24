<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Multiscale "Dream3DPipeline" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="dream3d" Label="Execute DREAM3D Pipeline" BaseType="operator">
      <ItemDefinitions>
        <File Name="point-file" Label="Point File" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="DEFORM-2D V6.0+ Point Tracking Output Data (*.rst *.RST);;All files (*.*)">
          <BriefDescription>DEFORM point tracking file</BriefDescription>
        </File>
        <File Name="step-file" Label="Step File" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="DEFORM-2D V6.0+ Step File (*.dat *.DAT);;All files (*.*)">
          <BriefDescription>DEFORM step file</BriefDescription>
        </File>
        <File Name="pipeline-executable" Label="Pipeline Executable" NumberOfRequiredValues="1" ShouldExist="true">
          <BriefDescription>Dream3D PipelineRunner executable</BriefDescription>
        </File>
        <File Name="stats-files" Label="Statistics Files" NumberOfRequiredValues="1" ShouldExist="true" Extensible="1"
              FileFilters="DREAM3D data file (*.dream3d);;All files (*.*)">
          <BriefDescription>stats generator data containers</BriefDescription>
	</File>
        <File Name="output-file" NumberOfRequiredValues="1" ShouldExist="false"
              FileFilters="DREAM3D data file (*.dream3d)">
          <BriefDescription>DREAM3D (xdmf) output file</BriefDescription>
	</File>
	<String Name="attribute">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="EffectiveStrainRates">EffectiveStrainRates</Value>
            <Value Enum="EffectiveStrains">EffectiveStrains</Value>
            <Value Enum="EffectiveStresses">EffectiveStresses</Value>
            <Value Enum="StrainRates">StrainRates</Value>
            <Value Enum="Strains">Strains</Value>
            <Value Enum="Stresses">Stresses</Value>
          </DiscreteInfo>
          <BriefDescription>attribute to use for zoning</BriefDescription>
	</String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(dream3d)" BaseType="result">
      <ItemDefinitions>
        <!-- The model read from the file. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
