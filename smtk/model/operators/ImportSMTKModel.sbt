<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ImportSMTKModel" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import smtk model" BaseType="operator">
      <BriefDescription>
        Import a JSON description of smtk model(s).
      </BriefDescription>
      <DetailedDescription>
        Import models in SMTK's native JSON format.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="SMTK JSON Model (*.smtk *.json);;All files (*.*)">
        </File>
        <Void Name="loadmesh" Label="Load Analysis Mesh" AdvanceLevel="1" Optional="true" IsEnabledByDefault="true" NumberOfRequiredValues="0">
          <BriefDescription>Specify whether related analysis meshes (if exist) should be loaded with model </BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(import smtk model)" BaseType="result">
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
