<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "Write" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write" Label="Mesh - Save" BaseType="operation">
      <BriefDescription>
        Write a mesh to disk.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Write a mesh to disk.
        &lt;p&gt;This operator creates a file representing the
        selected mesh and saves it to disk. The created file is
        readable by this application, and the resulting
        mesh is guaranteed to contain all of the information from
        the original mesh.
      </DetailedDescription>
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1"
                       Extensible="false" LockType="Read" OnlyResources="true">
        <Accepts><Resource Name="smtk::mesh::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          FileFilters="[defined programatically]">
        </File>
        <Int Name="write-component" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Entire Resource">0</Value>
            <Value Enum="Only Domain">1</Value>
            <Value Enum="Only Dirichlet">2</Value>
            <Value Enum="Only Neumann">3</Value>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
