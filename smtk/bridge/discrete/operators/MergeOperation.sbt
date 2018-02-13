<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Merge" Operation -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="merge face" BaseType="operation" Label="Face - Merge">
      <BriefDescription>
        Merge several faces into one face.
      </BriefDescription>
      <DetailedDescription>
        Merge several faces into one face.
      </DetailedDescription>
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
           <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <ModelEntity Name="source cell" NumberOfRequiredValues="0" Extensible="1">
          <BriefDescription>
            Source cell is what would be merged into Target cell.
          </BriefDescription>
          <DetailedDescription>
            Source cell is what would be merged into Target cell.

            Mutiple cells are allowed to be treated as source cells.
          </DetailedDescription>
          <MembershipMask>face</MembershipMask>
        </ModelEntity>

        <ModelEntity Name="target cell" NumberOfRequiredValues="1">
          <BriefDescription>
            Target cell is what source cells would be merged into.
          </BriefDescription>
          <DetailedDescription>
            Target cell is what source cells would be merged into.
          </DetailedDescription>
          <MembershipMask>face</MembershipMask>
        </ModelEntity>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(merge face)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeSystem>
