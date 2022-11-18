<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the markup "dump graph" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="dump graph" Label="dump a markup graph" BaseType="operation">

      <BriefDescription>Dump a graph of a markup resource's nodes.</BriefDescription>
      <AssociationsDef Name="resource" NumberOfRequiredValues="1" LockType="Read" OnlyResources="true">
        <BriefDescription>The resource to dump.</BriefDescription>
        <Accepts><Resource Name="smtk::markup::Resource" /></Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          Optional="true" IsEnabledByDefault="false"
          FileFilters="Graphviz files (*.dot)">
          <DefaultValue> </DefaultValue>
          <BriefDescription>A filename for the output; if disabled, dump to the console.</BriefDescription>
        </File>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(dump graph)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
