<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Parameters -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="ungroup" Label="ungroup" BaseType="operation">
      <BriefDescription>Dissolve a group.</BriefDescription>
      <DetailedDescription>Remove all of a group's members, then delete the group.</DetailedDescription>

      <AssociationsDef LockType="Write" NumberOfRequiredValues="1" Extensible="true">
        <BriefDescription>The group to dissolve.</BriefDescription>
        <Accepts>
          <Resource Name="smtk::markup::Resource" Filter="'smtk::markup::Group'"/>
        </Accepts>
      </AssociationsDef>

      <ItemDefinitions>
      </ItemDefinitions>

    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(ungroup)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeResource>
