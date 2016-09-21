<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Read" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="read" BaseType="operator">
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="OpenCascade Boundary Representation (*.brep *.occ);;All files (*.*)">
        </File>
        <String Name="filetype" NumberOfRequiredValues="1">
          <DefaultValue></DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(read)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
