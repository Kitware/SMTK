<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "DivideInstance" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="divide instance" BaseType="operation"
      Label="Model Entities - Divide Instance">
      <AssociationsDef
        Name="instance"
        NumberOfRequiredValues="1"
        Extensible="true"
        HoldReference="true">
        <Accepts><Resource Name="smtk::model::Resource" Filter="instance"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Divide an instance into multiple instances.
      </BriefDescription>
      <DetailedDescription>
        Divide an instance into multiple instances.

        An instance is a model entity whose geometry is a transformed
        duplicate of some other model entity; that source entity is
        called its prototype. Select a subset of instance placements
        using point or cell selection and apply this operation in
        order to divide the instance into two (the selected subset
        and the remainder). It is possible to create multiple selections
        to divide an instance into multiple pieces at once.
      </DetailedDescription>
      <ItemDefinitions>

        <Void
          Name="merge selections"
          IsEnabledByDefault="false"
          AdvanceLevel="2">
          <BriefDescription>Should all selected placements create a single output.</BriefDescription>
          <DetailedDescription>
            By default, each separate selected subset of the involved model instance
            results in a separate output instance. Setting this item to true will
            cause them to be merged so that the output contains only two instances:
            one holding unselected placements and one holding selected placements.
          </DetailedDescription>
        </Void>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(divide instance)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
