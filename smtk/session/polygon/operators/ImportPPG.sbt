<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB polygon Model "import" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Specification -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import-ppg" Label="Polygonal Planar Model from PPG File" BaseType="operation">
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Planar Polygon Files (*.ppg);;All files (*.*)">
          <BriefDescription>Input file listing vertex and face specifications.</BriefDescription>
        </File>
        <String Name="string" Label="String Input" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <BriefDescription>Intended for testing, overrides filename item</BriefDescription>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>

        <!-- The model resource created by the operation. -->
        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::session::polygon::Resource"/>
          </Accepts>
        </Resource>
      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeResource>
