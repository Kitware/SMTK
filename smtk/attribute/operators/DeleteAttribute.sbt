<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "delete attribute" Operation -->
<SMTK_AttributeResource Version="8" DisplayHint="true">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="delete attribute" Label="Delete Attribute" BaseType="operation">
      <BriefDescription>
        Delete the associated attribute(s).
      </BriefDescription>
      <AssociationsDef Name="attributes" NumberOfRequiredValues="0" Extensible="true">
        <Accepts><Resource Name="smtk::attribute::Resource" Filter="attribute"/></Accepts>
        <BriefDescription>
          The attribute(s) to delete.
        </BriefDescription>
      </AssociationsDef>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(delete attribute)" BaseType="result">
      <!-- ItemDefinitions>
        <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
      </ItemDefinitions -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
