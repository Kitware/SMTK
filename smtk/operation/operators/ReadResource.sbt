<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ReadResource" Operation -->
<SMTK_AttributeResource Version="3">

  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="read resource" Label="Read" BaseType="operation">
      <BriefDescription>
        Load one or more SMTK resources from disk
      </BriefDescription>
      <DetailedDescription>
        Load resources saved in SMTK's native JSON format.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" Extensible="true"
          FileFilters="SMTK Resource (*.smtk)" Label="SMTK Resource File Name " ShouldExist="true">
          <BriefDescription>The filename to load.</BriefDescription>
        </File>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(read resource)" BaseType="result"/>
  </Definitions>

</SMTK_AttributeResource>
