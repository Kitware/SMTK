<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Parameters -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create group" Label="group" BaseType="operation">

      <AssociationsDef LockType="Write" NumberOfRequiredValues="1" Extensible="true">
        <BriefDescription>The components to group.</BriefDescription>
        <Accepts>
          <Resource Name="smtk::markup::Resource" Filter="*"/>
        </Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <String Name="name" Label="name">
          <BriefDescription>The name of the group.</BriefDescription>
          <DefaultValue>new group</DefaultValue>
        </String>

      </ItemDefinitions>

    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create group)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeResource>
