<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB polygon Model "write" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write" Label="Model - Write Resource" BaseType="operation">
      <ItemDefinitions>
        <Resource Name="resource">
          <Accepts>
            <Resource Name="smtk::bridge::polygon::Resource"/>
          </Accepts>
        </Resource>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
