<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Exodus "EulerCharacteristicRatio" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="euler characteristic ratio" Label="Model - Compute Euler Characteristic Ratio" BaseType="operation">
      <BriefDescription>
        Compute the ratio of the Euler characteristics for a model's
        boundary to its volume.
      </BriefDescription>
      <AssociationsDef Name="Model" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::bridge::mesh::Resource" Filter="model"/></Accepts>
      </AssociationsDef>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(euler characteristic ratio)" BaseType="result">
      <ItemDefinitions>
        <Double Name="value" Label="Euler Characteristic Ratio" NumberOfRequiredValues="1" Extensible="false"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
