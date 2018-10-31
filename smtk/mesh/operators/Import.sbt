<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB mesh "read" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import" Label="Mesh - Import Resource" BaseType="operation">
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="[defined programatically]">
        </File>

        <String Name="label" Label="Domain Property" NumberOfRequiredValues="1" AdvanceLevel="1">
	  <DefaultValue></DefaultValue>
	</String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>

        <Resource Name="resource">
          <Accepts>
            <Resource Name="smtk::mesh::Collection"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
