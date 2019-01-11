<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "associate" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="mark modified"
            Label="Resource - Mark as Modified" BaseType="operation">
      <BriefDescription>
        Mark the specified components as created, modified, or expunged.
      </BriefDescription>
      <ItemDefinitions>
        <Component Name="created" Extensible="true" NumberOfRequiredValues="0">
          <Accepts><Resource Name="smtk::attribute::Resource" Filter="*"/></Accepts>
        </Component>
        <Component Name="modified" Extensible="true" NumberOfRequiredValues="0">
          <Accepts><Resource Name="smtk::attribute::Resource" Filter="*"/></Accepts>
        </Component>
        <Component Name="expunged" Extensible="true" NumberOfRequiredValues="0">
          <Accepts><Resource Name="smtk::attribute::Resource" Filter="*"/></Accepts>
        </Component>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(mark modified)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
