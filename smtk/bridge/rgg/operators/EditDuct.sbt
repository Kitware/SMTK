<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "EditDuct" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit duct" Label="Model - Edit Duct" BaseType="operator">
      <BriefDescription>Edit a RGG Duct.</BriefDescription>
      <DetailedDescription>
        User can edit an existing duct by changing its properties.
        Its pitch and height are pre-defined
        in the core.
      </DetailedDescription>
      <AssociationsDef Name="duct" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>aux_geom</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned name for the nulcear duct</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nulcear duct.
          </DetailedDescription>
        </String>
        <Void Name="cross section" NumberOfRequiredValues="1" Optional = "true" IsEnabledByDafault="false" AdvanceLevel="11">
          <BriefDescription>Cut away the duct so that the inner structure is visible</BriefDescription>
          <DetailedDescription>
            If enabled, SMTK would use a clipping plane that is perpendicular to the bottom face and goes through the base radius
            to cut the duct. By doing so, user is able to view the inner structure.
          </DetailedDescription>
        </Void>
        <Group Name="duct segments" Extensible="true" NumberOfRequiredGroups="1" AdvanceLevel="11">
          <BriefDescription>Segment the duct along the height into several pieces</BriefDescription>
          <DetailedDescription>
            Segment the duct along the height into several pieces. Each piece is defined by a base z value(Z1)
            and height(Z2 - Z1). Each piece would have its own materials layers as many as needed.
          </DetailedDescription>
          <ItemDefinitions>
            <Double Name="z values" NumberOfRequiredValues="2" AdvanceLevel="11">
              <BriefDescription>Z1 and Z2 value</BriefDescription>
              <DetailedDescription>
                Z1 and Z2 value.
              </DetailedDescription>
            </Double>
            <Int Name="materials" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
              <BriefDescription>A user assigned material for a nulcear duct layer</BriefDescription>
              <DetailedDescription>
                A user assigned material for a nulcear duct layer.
              </DetailedDescription>
            </Int>
            <Double Name="thicknesses(normalized)" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
              <BriefDescription>A user assigned thicknesses for a nulcear duct layer</BriefDescription>
              <DetailedDescription>
                A user assigned thicknesses for a nulcear duct layer.
                If the duct is hex, then each material would have one thickeness along radius(same thickeness along x and y axis).
                If the duct is rectilinear, then each material would have two thicknesses along width and length.
              </DetailedDescription>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(edit duct)" BaseType="result">
      <ItemDefinitions>
        <!-- The edit duct is returned in the base result's "edit" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkRGGEditDuctView ...)
      -->
    <View Type="smtkRGGEditDuctView" Title="Edit Duct"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="false">
      <Description>
        TODO: Add documentation for edit duct operator.
      </Description>
      <AttributeTypes>
        <Att Type="edit duct"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
