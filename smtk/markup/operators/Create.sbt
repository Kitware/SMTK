<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the markup resource's "write" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create" Label="create markup" BaseType="operation">
      <ItemDefinitions>
        <File Name="filename" Label="filename"
          ShouldExist="false" Optional="true" IsEnabledByDefault="false"
          FileFilters="SMTK Files (*.smtk)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
