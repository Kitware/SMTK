<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Read" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="read" BaseType="operator">
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Conceptual Model Builder (*.cmb);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(read)" BaseType="result">
      <ItemDefinitions>
        <!-- The model read from the file. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
