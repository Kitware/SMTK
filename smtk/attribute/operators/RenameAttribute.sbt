<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "rename attribute" Operation -->
<SMTK_AttributeResource Version="8" DisplayHint="true">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="rename attribute" Label="Rename Attribute" BaseType="operation">
      <BriefDescription>
        Rename the associated attribute(s).
      </BriefDescription>
      <AssociationsDef Name="attribute" NumberOfRequiredValues="0" Extensible="true">
        <Accepts><Resource Name="smtk::attribute::Resource" Filter="attribute"/></Accepts>
        <BriefDescription>
          The attribute(s) to rename.
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="0" Extensible="true">
          <BriefDescription>
            The desired attribute name(s).
          </BriefDescription>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(rename attribute)" BaseType="result">
      <!-- ItemDefinitions>
        <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
      </ItemDefinitions -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
