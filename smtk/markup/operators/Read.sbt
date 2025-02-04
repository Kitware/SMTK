<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the markup resource's "read" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="read" Label="read markup" BaseType="operation">
      <ItemDefinitions>
        <File Name="filename" Label="filename"
          ShouldExist="true"
          FileFilters="SMTK Files (*.smtk)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(read)" BaseType="result">
      <ItemDefinitions>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
