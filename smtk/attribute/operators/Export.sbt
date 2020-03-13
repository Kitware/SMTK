<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "export" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="export"
            Label="Attribute - Export" BaseType="operation">
      <AssociationsDef LockType="Read" OnlyResources="true">
        <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
      </AssociationsDef>
     <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1" ShouldExist="false"
              FileFilters="SMTK SimBuilder Instance Files (*.sbi);;All files (*.*)">
        </File>
        <!-- Advanced option to export specified attribute types for reuse in other resources -->
        <Group Name="attribute-collection" Label="Selected Attribute Types" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false">
          <ItemDefinitions>
            <String Name="types" Label="Attribute Types" Extensible="true" NumberOfRequiredValues="1" />
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
     </AttDef>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(export)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
