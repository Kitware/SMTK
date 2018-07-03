<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "Mesh" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="mesh" BaseType="operation" AdvanceLevel="11">
      <AssociationsDef NumberOfRequiredValues="1" Name="model">
        <Accepts><Resource Name="smtk::model::Manager" Filter="model"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="endpoint" NumberOfRequiredValues="1"/>
        <String Name="remusRequirements" NumberOfRequiredValues="1"/>
        <String Name="meshingControlAttributes" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(mesh)" BaseType="result">
      <ItemDefinitions>
        <Component Name="mesh_created" NumberOfRequiredValues="1">
          <Accepts><Resource Name="smtk::model::Manager" Filter="model"/></Accepts>
        </Component>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
