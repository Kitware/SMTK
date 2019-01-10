<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "associate" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="mark modified"
            Label="Resource - Mark as Modified" BaseType="operation">
      <BriefDescription>
        Mark resources owning the associated components as modified.
      </BriefDescription>
      <AssociationsDef Name="components" Extensible="true" NumberOfRequiredValues="0">
        <Accepts><Resource Name="smtk::resource::Resource" Filter="*"/></Accepts>
      </AssociationsDef>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(mark modified)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
