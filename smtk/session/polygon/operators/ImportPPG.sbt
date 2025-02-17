<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB polygon Model "import" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Specification -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import-ppg" Label="Planar Polygon Model from PPG File" BaseType="operation">
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Planar Polygon Files (*.ppg)">
          <BriefDescription>Input file listing vertex and face specifications.</BriefDescription>
        </File>
        <String Name="string" Label="String Input" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <BriefDescription>Intended for testing, overrides filename item</BriefDescription>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
