<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the markup resource's "edit comment" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="edit comment" BaseType="operation">
      <AssociationsDef LockType="Write" Extensible="true">
        <Accepts>
          <!-- Accept any markup component -->
          <Resource Name="smtk::markup::Resource" Filter="*"/>
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="text">
          <BriefDescription>The text of the comment to add or update.</BriefDescription>
          <DefaultValue></DefaultValue>
        </String>
        <String Name="mime-type" AdvanceLevel="1">
          <BriefDescription>The type of data held in text.</BriefDescription>
          <DefaultValue>text/plain</DefaultValue>
        </String>
        <Void Name="remove when empty" Optional="true" IsEnabledByDefault="true">
          <BriefDescription>When the comment text is empty and a comment is being edited (rather than created), the comment will be deleted when this item is enabled.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(edit comment)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
