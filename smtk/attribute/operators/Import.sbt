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
        <!-- Advanced option to import into existing resource instead of creating new one -->
        <Resource Name="use-resource" Label="Use Existing Resource" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
        </Resource>
      <Void Name="UseDirectoryInfo" Label="Use Directory Information"
            Optional="true" IsEnabledByDefault="true"/>
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
