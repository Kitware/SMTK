<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Mesh "Import" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import" Label="Model - Import" BaseType="operation">
      <BriefDescription>
        Import a model from a mesh file.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Import a model from a mesh file.
        &lt;p&gt;This operator imports a mesh into smtk, and then
        parses its components by dimension and connectivity to form a model.
      </DetailedDescription>

      <!-- Import operations can import a file into an existing
           resource (or an existing resource's session) if one is
           provided. Otherwise, a new resource is created -->
      <AssociationsDef Name="import into" NumberOfRequiredValues="0"
                       Extensible="true" MaxNumberOfValues="1" OnlyResources="true">
        <Accepts><Resource Name="smtk::session::mesh::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="[defined programatically]">
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

        <String Name="label" Label="Domain Property" NumberOfRequiredValues="1" AdvanceLevel="1">
          <DefaultValue></DefaultValue>
        </String>

        <Void Name="construct hierarchy" Label="Construct Model Hierarchy" Optional="true" IsEnabledByDefault="false"/>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>
        <Component Name="model">
          <Accepts>
            <Resource Name="smtk::session::mesh::Resource" Filter=""/>
          </Accepts>
        </Component>

        <Component Name="mesh_created">
          <Accepts>
            <Resource Name="smtk::session::mesh::Resource" Filter=""/>
          </Accepts>
        </Component>

        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>

      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
