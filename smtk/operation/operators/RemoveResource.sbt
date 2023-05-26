<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "RemoveResource" Operation -->
<SMTK_AttributeResource Version="3">

  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="remove resource" Label="Remove" BaseType="operation">
      <BriefDescription>
        Remove one or more SMTK resources
      </BriefDescription>
      <DetailedDescription>
        Remove resources from its associated resource manager.
      </DetailedDescription>
      <AssociationsDef LockType="Write" HoldReference="true"
                       OnlyResources="true" Extensible="true">
        <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Void Name="removeAssociations" Label="Remove Associations" Optional="True" IsEnabledByDefault="false">
          <BriefDescription>
            Remove any associations to removed resources, clearing any reference items refering to removed resources.
          </BriefDescription>
        </Void>
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
