<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Read" Operator -->
<SMTK_AttributeManager Version="1">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="read" BaseType="operator">
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"/>
        <String Name="filetype" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(read)" BaseType="result">
      <ItemDefinitions>
        <!-- The model read from the file. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeManager>
