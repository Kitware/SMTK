<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the OpenCASCADE "create resource" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create resource" Label="Model - Create" BaseType="operation">

      <ItemDefinitions>
        <File Name="location" Label="File" NumberOfRequiredValues="1"
          Optional="true" IsEnabledByDefault="false" ShouldExist="false">
          <BriefDescription>
            A filename to use for the new resource.
          </BriefDescription>
          <DetailedDescription>
            Provide a default filename for the resource.
            If none is provided, the default is simply "New Resource",
            possibly followed by a number to make it unique, and will be placed
            in your document directory.
          </DetailedDescription>
        </File>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create resource)" BaseType="result">
      <ItemDefinitions>

        <!-- The model imported from the file. -->
        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::session::opencascade::Resource"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
