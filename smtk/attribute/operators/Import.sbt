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
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="SMTK SimBuilder Template Files (*.sbt);;SMTK SimBuilder Instance Files (*.sbi);;eXtensible Markup Language files (*.xml*)">
        </File>
      <Void Name="UseDirectoryInfo" Label="Use Directory Information"
            Optional="true" IsEnabledByDefault="false"/>
      </ItemDefinitions>
    </AttDef>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>

        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::attribute::Resource"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
