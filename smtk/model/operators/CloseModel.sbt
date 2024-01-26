<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "CloseModel" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="close model" Label="Model - Close" BaseType="operation">
      <BriefDescription>
        Close the associated models.
      </BriefDescription>
      <DetailedDescription>
        Close the associated models.
        This will permanently discard any changes you may have made to the models.
        Save the models first if you want to keep any changes.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="true">
        <Accepts><Resource Name="smtk::model::Resource" Filter="model"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Void Name="show non-active models" AdvanceLevel= "11">
          <BriefDescription> Show non-active models which belongs to current active model's session in available models combobox.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(close model)" BaseType="result">
      <!-- The close models are stored in the base result's "expunged" item. -->
      <ItemDefinitions>
        <Resource Name="resource modified" IsEnabledByDefault="true" Extensible="true" NumberOfRequiredValues="0" />
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
