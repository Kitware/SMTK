<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "create attribute" Operation -->
<SMTK_AttributeResource Version="8" DisplayHint="true">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create attribute" Label="Create Attribute" BaseType="operation">
      <BriefDescription>
        Create an attribute of the given type.
      </BriefDescription>
      <AssociationsDef Name="resource" OnlyResources="true">
        <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
        <BriefDescription>
          The resource in which to create the resource.
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="definition">
          <BriefDescription>
            The name of a concrete attribute definition type to create.
          </BriefDescription>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create attribute)" BaseType="result">
      <!-- ItemDefinitions>
        <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
      </ItemDefinitions -->
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
