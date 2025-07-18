<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the markup "import" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import" Label="import data" BaseType="operation">

      <!-- Import operations can import a file into an existing
           resource (or an existing resource's session) if one is
           provided. Otherwise, a new resource is created -->
      <AssociationsDef Name="import into" NumberOfRequiredValues="0"
                       Extensible="true" MaxNumberOfValues="1" OnlyResources="true">
        <Accepts><Resource Name="smtk::markup::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <File Name="filename" Label="filename" NumberOfRequiredValues="1"
          ShouldExist="true" Extensible="true"
          FileFilters="VTK Unstructured Grids (*.vtu);; VTK Polydata (*.vtp);; VTK Image Data (*.vti);; Web Ontology Language (*.owl);;">
        </File>

        <Void Name="consistency" Label="enforce consistency" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>If enabled, force surfaces to have consistent, outward-pointing normals.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <include href="smtk/operation/Hints.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>
        <Void Name="allow camera reset" Optional="true" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
