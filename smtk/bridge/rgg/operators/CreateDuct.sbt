<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "CreateDuct" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create duct" Label="Model - Create Duct" BaseType="operator">
      <BriefDescription>Create a RGG Duct.</BriefDescription>
      <DetailedDescription>
        By providing a name user can create a simple duct. Its pitch and length should be pre-defined
        in the core(TBD). For now user can edit them.
        After the creation, CMB would automatically switch to "Edit Duct" operator
        so that user can tweak other properties.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="0">
          <BriefDescription>A user assigned name for the nulcear duct</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nulcear duct.
          </DetailedDescription>
          <DefaultValue>Duct0</DefaultValue>
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
              <DefaultValue>0.0, 0.0</DefaultValue>
            </Double>
            <Int Name="materials" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
              <BriefDescription>A user assigned material for a nulcear duct layer</BriefDescription>
              <DetailedDescription>
                A user assigned material for a nulcear duct layer.
              </DetailedDescription>
              <DefaultValue>0</DefaultValue>
            </Int>
            <Double Name="thicknesses(normalized)" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
              <BriefDescription>A user assigned thicknesses for a nulcear duct layer</BriefDescription>
              <DetailedDescription>
                A user assigned thicknesses for a nulcear duct layer.
                If the duct is hex, then each material would have one thickeness along radius(same thickeness along x and y axis).
                If the duct is rectilinear, then each material would have two thicknesses along width and length.
              </DetailedDescription>
              <DefaultValue>1.0</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create duct)" BaseType="result">
      <ItemDefinitions>
        <!-- The created duct is returned in the base result's "create" item. -->
        <Void Name="force camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
        <Void Name="hide other entities" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
