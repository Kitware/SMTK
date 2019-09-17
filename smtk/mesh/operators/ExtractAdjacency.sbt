<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "ExtractAdjacency" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="extract adjacency" BaseType="operation" Label="Mesh - Extract Adjacency">
      <BriefDescription>
        Extract a mesh's adjacency mesh.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Extract a mesh's adjacency mesh.
        &lt;p&gt;The user can select the dimension of the returned adjacency mesh.
      </DetailedDescription>
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <!-- TODO: Can int items have a combobox representation? -->
        <String Name="dimension" Label="Dimension" NumberOfRequiredValues="1">
          <BriefDescription>Dimension of the adjacency mesh</BriefDescription>
          <DiscreteInfo DefaultIndex="2">
              <Value Enum="0">0</Value>
              <Value Enum="1">1</Value>
              <Value Enum="2">2</Value>
              <Value Enum="3">3</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(extract adjacency)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
