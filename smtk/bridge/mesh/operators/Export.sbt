<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Mesh Session "Export" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write" Label="Model - Export" BaseType="operation">
      <AssociationsDef Name="Model(s)" NumberOfRequiredValues="1" Extensible="true">
        <Accepts><Resource Name="smtk::bridge::mesh::Resource" Filter="model"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="false"
          FileFilters="Moab files (*.h5m);;Exodus II Datasets (*.e *.exo *.ex2);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
