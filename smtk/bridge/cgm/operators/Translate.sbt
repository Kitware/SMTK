<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "BooleanUnion" Operation -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="translate" BaseType="operation">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <Accepts><Resource Name="smtk::bridge::cgm::Resource" Filter="model"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="offset" NumberOfRequiredValues="3">
          <DefaultValue>0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(translate)" BaseType="result">
      <!-- The translated body (or bodies) are stored in the base result's "modified" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
