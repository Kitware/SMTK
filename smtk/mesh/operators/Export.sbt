<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="export" Label="Mesh - Export" BaseType="operation">
      <BriefDescription>
        Export a mesh to disk.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Export a mesh to disk.
        &lt;p&gt;This operator creates a file representing the
        selected mesh and saves it to disk. The created file is
        readable by this application, but the resulting
        mesh is not guaranteed to contain all of the information from
        the original mesh.
      </DetailedDescription>
      <AssociationsDef Name="mesh" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          FileFilters="[defined programatically]">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(export)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
