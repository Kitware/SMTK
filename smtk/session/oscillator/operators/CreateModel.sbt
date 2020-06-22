<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the oscillator "CreateModel" Operator -->
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Operator -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create model" Label="Oscillator Model" BaseType="operation">
      <BriefDescription>Create an oscillator model.</BriefDescription>
      <DetailedDescription>
        Create a geometric oscillator model.
        Each model may have 0 or more domains (uniform grids) and 0 or more
        source terms. Domains specify the regions of space  over which
        contributions from oscillator sources are summed.
      </DetailedDescription>
      <!-- We may, but are not required to, add this model to an existing resource -->
      <AssociationsDef Name="resource" NumberOfRequiredValues="0" Extensible="true">
        <Accepts>
          <Resource Name="smtk::session::oscillator::Resource" Filter=""/>
        </Accepts>
      </AssociationsDef>
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
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create model)" BaseType="result">
      <ItemDefinitions>
        <!-- The created model is returned in the base result's "created" item. -->
        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::session::oscillator::Resource"/>
          </Accepts>
        </Resource>
      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeResource>
