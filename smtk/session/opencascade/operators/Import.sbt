<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the OpenCASCADE "import" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import" Label="Model - Import Geometry" BaseType="operation">

      <!-- Import operations can import a file into an existing
           resource (or an existing resource's session) if one is
           provided. Otherwise, a new resource is created -->
      <AssociationsDef Name="import into" NumberOfRequiredValues="0"
                       Extensible="true" MaxNumberOfValues="1" OnlyResources="true">
        <Accepts><Resource Name="smtk::session::opencascade::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true" Extensible="true"
          FileFilters="Supported Files (*.brep *.step *.stp *.iges *.igs);; OpenCascade Boundary Representation Data (*.brep);; STEP Files (*.step *.stp);; IGES Files (*.iges *.igs);; All files (*.*)">
        </File>

      <!-- In the event that we are importing into an existing
           resource, this enumeration allows the user to select
           whether the import should simply use the resource's session
           or if the imported model should be a part of the resource
           itself -->
        <String Name="session only" Label="session" AdvanceLevel="1">
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="this file">import into this file</Value>
            </Structure>
            <Structure>
              <Value Enum="this session">import into a new file using this file's session</Value>
            </Structure>
          </DiscreteInfo>
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>

        <!-- The model imported from the file. -->
        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::session::opencascade::Resource"/>
          </Accepts>
        </Resource>

        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
