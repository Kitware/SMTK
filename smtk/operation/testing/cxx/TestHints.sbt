<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="6">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="MyOperation" Label="My Operation" BaseType="operation">
      <ItemDefinitions>
        <Int Name="sleep" Optional="False">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(test op)" BaseType="result">
      <ItemDefinitions>
        <Resource Name="resource" HoldReferences="True">
          <Accepts Resource="smtk::model::Resource"/>
        </Resource>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
