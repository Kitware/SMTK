<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB mesh "read" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="read" Label="Mesh - Read Resource" BaseType="operation">
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="SMTK Files (*.smtk)">
        </File>

        <Int Name="subset" Label="Mesh Subset"
             NumberOfRequiredValues="1" AdvanceLevel="1">
          <BriefDescription>Subset of the mesh to load</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Entire Resource">0</Value>
            <Value Enum="Only Domain">1</Value>
            <Value Enum="Only Dirichlet">2</Value>
            <Value Enum="Only Neumann">3</Value>
          </DiscreteInfo>
        </Int>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(read)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
