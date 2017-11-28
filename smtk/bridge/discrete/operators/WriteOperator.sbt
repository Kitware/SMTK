<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Read" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="write" BaseType="operator" Label="Model - Write">
      <AssociationsDef Name="Model" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
<!--
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
-->
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="false"
          FileFilters="Conceptual Model Builder (*.cmb)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
