<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Merge" Operation -->
<SMTK_AttributeResource Version="3">
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
      <AssociationsDef Name="source cell" NumberOfRequiredValues="1" Extensible="1">
        <BriefDescription>
          Source cell is what would be merged into Target cell.
        </BriefDescription>
        <DetailedDescription>
          Source cell is what would be merged into Target cell.

          Mutiple cells are allowed to be treated as source cells.
        </DetailedDescription>
        <Accepts>
          <Resource Name="smtk::session::discrete::Resource" Filter="face"/>
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Component Name="model" NumberOfRequiredValues="1">
           <Accepts>
             <Resource Name="smtk::session::discrete::Resource" Filter="model"/>
           </Accepts>
        </Component>

        <Component Name="target cell" NumberOfRequiredValues="1">
          <BriefDescription>
            Target cell is what source cells would be merged into.
          </BriefDescription>
          <DetailedDescription>
            Target cell is what source cells would be merged into.
          </DetailedDescription>
           <Accepts>
             <Resource Name="smtk::session::discrete::Resource" Filter="face"/>
           </Accepts>
        </Component>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(merge face)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeResource>
