<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "SetInstancePrototype" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="set instance prototype" BaseType="operation"
      Label="Model Entities - Change Instance Prototype">
      <AssociationsDef Name="instance" NumberOfRequiredValues="0" Extensible="true">
        <Accepts><Resource Name="smtk::model::Resource" Filter="instance"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Change an instance's prototype geometry.
      </BriefDescription>
      <DetailedDescription>
        Change an instance's prototype geometry.

        An instance is a model entity whose geometry is a transformed
        duplicate of some other model entity; that source entity is
        called its prototype. Choose any entity in the same resource
        as long as it has a tessellation of its own.
      </DetailedDescription>
      <ItemDefinitions>

        <Component
          Name="prototype"
          NumberOfRequiredValues="1">
          <BriefDescription>The entity to use as source geometry.</BriefDescription>
          <DetailedDescription>
            A model entity to use as the prototype for this instance's source geometry.
          </DetailedDescription>
          <Accepts><Resource Name="smtk::model::Resource" Filter="any"/></Accepts>
        </Component>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(set instance prototype)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
