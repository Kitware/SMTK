<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "WriteResource" Operation -->
<SMTK_AttributeResource Version="3">

  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write resource" Label="Write" BaseType="operation">
      <BriefDescription>
        Write one or more SMTK resources to disk
      </BriefDescription>
      <DetailedDescription>
        Write resources in SMTK's native JSON format.
        Resources are written to their existing location unless
        the "filename" item is enabled and set to a valid value.
      </DetailedDescription>
      <AssociationsDef LockType="Read" OnlyResources="true">
        <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" Extensible="true" Optional="true"
          FileFilters="SMTK Resource (*.smtk)" Label="SMTK Resource File Name " ShouldExist="false">
          <BriefDescription>The destination filename.</BriefDescription>
        </File>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write resource)" BaseType="result">
      <ItemDefinitions>
        <File Name="files" NumberOfRequiredValues="0"
              Extensible="true" ShouldExist="true">
        </File>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

</SMTK_AttributeResource>
