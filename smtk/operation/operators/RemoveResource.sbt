<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "RemoveResource" Operation -->
<SMTK_AttributeResource Version="3">

  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="remove resource" Label="Remove" BaseType="operation">
      <BriefDecscription>
        Remove one or more SMTK resources
      </BriefDecscription>
      <DetailedDecscription>
        Remove resources from its associated resource manager.
      </DetailedDecscription>
      <AssociationsDef LockType="Write" HoldReference="true"
                       OnlyResources="true">
        <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(remove resource)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

</SMTK_AttributeResource>
