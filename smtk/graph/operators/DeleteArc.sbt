<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "delete arc type" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="delete arc" Label="connect nodes" BaseType="operation">

      <ItemDefinitions>
        <String Name="arc type">
          <BriefDescription>
            The type of arc to be deleted.
          </BriefDescription>
          <!--
            This string item is discrete and has conditional children
            enabled so that arc endpoint-types can be specific to arc types.
            The ChildrenDefinitions and DiscreteInfo sections are added as
            user-editable arc types are registered with this operation via
            class-static methods. (These methods are called by Create, Read,
            and CreateArcType operations.)
            -->
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(delete arc)" BaseType="result">
      <ItemDefinitions>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
