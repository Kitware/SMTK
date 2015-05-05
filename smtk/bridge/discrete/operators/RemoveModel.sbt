<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "CloseModel" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="remove model" BaseType="operator" AdvanceLevel="11">
      <AssociationsDef Name="model(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
    </AttDef>

    <!-- Result -->
    <AttDef Type="result(remove model)" BaseType="result">
      <!-- The close models are stored in the base result's "expunged" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
