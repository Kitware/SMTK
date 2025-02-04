<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "MarkModified" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="mark modified" BaseType="operation">
      <BriefDescription>
        Indicate that a resource has been modified (and is thus dirty, in need
        of saving to a file).
      </BriefDescription>
      <AssociationsDef Name="resources" NumberOfRequiredValues="0" Extensible="true">
        <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(mark modified)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
