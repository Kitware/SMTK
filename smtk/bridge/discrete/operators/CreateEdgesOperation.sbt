<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Create Edges" Operation -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create edges" BaseType="operation" Label="Edge - Create from face adjacencies">
      <BriefDescription>
        Given a discrete model, create edges and corresponding vertices.
      </BriefDescription>
      <DetailedDescription>
        Given a discrete model, create edges and corresponding vertices.

        Switch to Topolygy view first then hit apply button. Otherwise edges and vertices
        would not be created properly.
      </DetailedDescription>
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create edges)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
