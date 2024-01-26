<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "import" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import"
            Label="Attribute - Import" BaseType="operation">
      <BriefDescription>
        Import an attribute resource definition.
      </BriefDescription>

      <!-- Import operations can import a file into an existing
           resource if one is provided. Otherwise, a new resource is
           created -->
      <AssociationsDef Name="import into" NumberOfRequiredValues="0"
                       Extensible="true" MaxNumberOfValues="1" OnlyResources="true">
        <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="SMTK SimBuilder Template Files (*.sbt);;SMTK SimBuilder Instance Files (*.sbi);;eXtensible Markup Language files (*.xml*)">
        </File>
      <Void Name="UseDirectoryInfo" Label="Use Directory Information"
            Optional="true" IsEnabledByDefault="true"/>
      </ItemDefinitions>
    </AttDef>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>

        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::attribute::Resource" Extensible="true" NumberOfRequiredValues="0"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
