<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="select cells" Label="Mesh - Select Cells" BaseType="operation">
      <BriefDescription>
        Construct a mesh selection from a mesh resource and a list of
        cell ids.
      </BriefDescription>
      <AssociationsDef Name="mesh resource" NumberOfRequiredValues="1"
                       Extensible="false" OnlyResources="true">
        <Accepts><Resource Name="smtk::mesh::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <!-- TODO: support 64-bit integers in the attribute system -->
        <String Name="cell ids" NumberOfRequiredValues="0" Extensible="true"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(select cells)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
