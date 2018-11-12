<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "export_to_pyarc" Operator -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="export to pyarc" Label="Model - Export to PyARC" BaseType="operation">
      <BriefDescription>
        Export an RGG model as PyARC geometry
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Export an RGG model as PyARC geometry.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
          FileFilters="PyARC SON file (*.son)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(export to pyarc)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
