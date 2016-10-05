<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Exodus "Write" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="write" BaseType="operator">
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
    <AttDef Type="result(write)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
