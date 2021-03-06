<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB mesh Model "write" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="write" Label="Model - Write Resource" BaseType="operation">
      <AssociationsDef LockType="Read" OnlyResources="true">
          <Accepts><Resource Name="smtk::session::mesh::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>

        <Void Name="archive" Label="Archive files" Optional="true" IsEnabledByDefault="true" AdvanceLevel="1">
          <BriefDescription>
            Archive all related files into a single archive.
          </BriefDescription>
        </Void>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(write)" BaseType="result">
      <ItemDefinitions>
        <File Name="additional files" NumberOfRequiredValues="0"
              Extensible="true" ShouldExist="true">
        </File>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
