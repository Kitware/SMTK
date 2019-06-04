<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB vtk Model "read" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="read" Label="Model - Read Resource" BaseType="operation">
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="SMTK Files (*.smtk);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(read)" BaseType="result">
      <ItemDefinitions>

        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::session::vtk::Resource"/>
          </Accepts>
        </Resource>

        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
