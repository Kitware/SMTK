<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Exodus "Read" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="read" BaseType="operator">
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Exodus II Datasets (*.e *.exo *.ex2);;NetCDF files (*.nc *.ncdf);;All files (*.*)">
        </File>
        <String Name="filetype" NumberOfRequiredValues="1"/>
        <Int Name="readSLACVolumes" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <DiscreteInfo DefaultIndex="0">
            <Structure><Value Enum="no">0</Value></Structure>
            <Structure><Value Enum="yes">1</Value></Structure>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(read)" BaseType="result">
      <ItemDefinitions>
        <!-- The model read from the file. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
