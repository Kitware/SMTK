<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "CreateAssembly" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create assembly" Label="Model - Create Assembly" BaseType="operator">
      <BriefDescription>Create a RGG Assembly.</BriefDescription>
      <DetailedDescription>
        By providing a name user can create a simple Assembly. Its pitch and length should be
        pre-defined in the core(TBD). For now it's decided by duct.
        After the creation, CMB would automatically switch to "Edit Assembly" operator
        so that user can tweak other properties.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="0">
          <BriefDescription>A user assigned name for the nulcear assembly</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nulcear assembly.
          </DetailedDescription>
          <DefaultValue>assemblyEntity</DefaultValue>
        </String>
        <String Name="label" NumberOfRequiredValues="1" AdvanceLevel="0">
          <BriefDescription>A user assigned label for the nulcear assembly</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nulcear assembly.
          </DetailedDescription>
          <DefaultValue>assembly</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create assembly)" BaseType="result">
      <ItemDefinitions>
        <!-- The created assembly is returned in the base result's "create" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
