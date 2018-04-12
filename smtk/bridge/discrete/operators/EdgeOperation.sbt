<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "edge" Operation -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="modify edge" BaseType="operation" Label="Edge - Modify">
      <BriefDescription>
        Modify mesh edges by merging two edges into one or splitting one edge into two.
      </BriefDescription>
      <DetailedDescription>
        Modify mesh edges by merging two edges into one or splitting one edge into two.

        Edges can be merged or split, but not within the same operation. And merge operation
        (promotion) would always be processed before split operation(demotion). Currently
        "modify edge" operator only supports one merge/split per selection.

        Tips: Turn off the visibility of entity faces and mesh faces before operate.

      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1">
        <Accepts>
          <Resource Name="smtk::bridge::discrete::Resource" Filter="model"/>
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <MeshSelection Name="selection" ModelEntityRef="model">
          <MembershipMask>edge|vertex</MembershipMask>
          <BriefDescription>
            Select the mesh edge to split or mesh vertex to demote.
          </BriefDescription>
        </MeshSelection>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(modify edge)" BaseType="result">
      <ItemDefinitions>
        <!-- The modified entities are stored in the base result's "modified" item. -->
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
