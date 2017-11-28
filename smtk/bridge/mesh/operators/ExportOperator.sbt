<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Mesh Session "Export" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="write" Label="Model - Export" BaseType="operator">
      <AssociationsDef Name="Model(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="false"
          FileFilters="Moab files (*.h5m);;Exodus II Datasets (*.e *.exo *.ex2);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
