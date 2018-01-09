<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "CreateModel" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create model" Label="Model - Create" BaseType="operator">
      <BriefDescription>Create a RGG model.</BriefDescription>
      <DetailedDescription>
        Create a simple RGG model
      </DetailedDescription>
      <ItemDefinitions>
        <String Name="name" NumberOfValuesRequired="1" Optional="true">
          <BriefDescription>A user-assigned name for the model.</BriefDescription>
          <DetailedDescription>
            A user-assigned name for the model.
            The name need not be unique, but unique names are best.
            If not assigned, a machine-generated name will be assigned.
          </DetailedDescription>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create model)" BaseType="result">
      <ItemDefinitions>
        <!-- The created model is returned in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
