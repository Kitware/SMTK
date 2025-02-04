<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "read" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="read"
            Label="Attribute - Read Resource" BaseType="operation">
      <BriefDescription>
        Read an attribute resource.
      </BriefDescription>
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="SMTK Files (*.smtk);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(read)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
