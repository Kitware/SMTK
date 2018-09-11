<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "BooleanUnion" Operation -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="union" BaseType="operation">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="2" Extensible="true">
        <Accepts><Resource Name="smtk::session::cgm::Resource" Filter="model"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="keep inputs" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(union)" BaseType="result">
      <ItemDefinitions>
        <!-- The united body (or bodies) is return in the base result's "modified" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
