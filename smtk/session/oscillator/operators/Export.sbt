<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the oscillator "Export" Operator -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="export" Label="Oscillator - Export" BaseType="operation">
      <AssociationsDef Name="simulation" NumberOfRequiredValues="1"
                       Extensible="false" OnlyResources="true">
        <Accepts>
          <Resource Name="smtk::attribute::Resource"/>
        </Accepts>
        <BriefDescription>Choose an oscillator simulation attribute-resource that has
been linked to an oscillator simulation model-resource.
(The link is normally created for you during initialization.)
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="false"
          FileFilters="Oscillator configuration (*.osc);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(export)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
