<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Builder" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import" BaseType="operator">
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Legacy VTK files (*.vtk);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(import)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
