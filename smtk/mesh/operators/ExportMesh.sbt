<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="export mesh" Label="Mesh - Export" BaseType="operation">
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
      <ItemDefinitions>
        <MeshEntity Name="mesh" NumberOfRequiredValues="1" Extensible="true" />
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          FileFilters="AdH 2D Mesh file (*.2dm);;AdH 3D Mesh file (*.3dm)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(export mesh)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
