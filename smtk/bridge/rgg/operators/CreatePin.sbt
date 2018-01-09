<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "CreatePin" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create pin" Label="Model - Create Pin" BaseType="operator">
      <BriefDescription>Create a RGG Pin.</BriefDescription>
      <DetailedDescription>
        By providing a name and label user can create a simple pin.
        After the creation, CMB would automatically switch to "Edit pin" operator
        so that user can tweak other properties.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="0">
          <BriefDescription>A user assigned name for the nulcear pin</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nulcear pin.
          </DetailedDescription>
          <DefaultValue>PinCell</DefaultValue>
        </String>
        <String Name="label" NumberOfRequiredValues="1" AdvanceLevel="0">
          <BriefDescription>A user assigned label for the nulcear pin</BriefDescription>
          <DetailedDescription>
            A user assigned label for the nulcear pin.
          </DetailedDescription>
          <DefaultValue>rgg pin cell</DefaultValue>
        </String>
        <Int Name="cell material" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned outer material for the nulcear pin</BriefDescription>
          <DetailedDescription>
            A user assigned outer material for the nulcear pin.
          </DetailedDescription>
          <!-- no material -->
          <DefaultValue>0</DefaultValue>
        </Int>
        <Void Name="hex" NumberOfRequiredValues="1" Optional = "true" IsEnabledByDafault="false" AdvanceLevel="11">
          <BriefDescription>Create a hex nulcear pin</BriefDescription>
          <DetailedDescription>
            If enabled, SMTK create create a hex nulcear pin. Otherwise it would be a rectilinear nuclear pin.
          </DetailedDescription>
        </Void>
        <Void Name="cut away" NumberOfRequiredValues="1" Optional = "true" IsEnabledByDafault="false" AdvanceLevel="11">
          <BriefDescription>cut away the pin so that the inner layers are visible</BriefDescription>
          <DetailedDescription>
            If enabled, SMTK would use a clipping plane that is perpendicular to the bottom and goes through the base radius
            to cut the pin. By doing so, user is able to view the inner layers.
          </DetailedDescription>
        </Void>
        <Double Name="z origin" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned z origin for the nulcear pin</BriefDescription>
          <DetailedDescription>
            A user assigned z origin for the nulcear pin.
          </DetailedDescription>
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <Group Name="pieces" Extensible="true" NumberOfRequiredGroups="1" AdvanceLevel="11">
          <BriefDescription>A user assigned a set of sections which form the the nulcear pin from bottom to top</BriefDescription>
          <DetailedDescription>
             A user assigned a set of sections which form the the nulcear pin from bottom to top
          </DetailedDescription>
          <ItemDefinitions>
            <Int Name="segment type" NumberOfRequiredValues="1" AdvanceLevel="11">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Cylinder">0</Value>
                <Value Enum="Frustum">1</Value>
              </DiscreteInfo>
            </Int>
            <Double Name="type parameters" NumberOfRequiredValues="3" AdvanceLevel="11">
              <!-- length, base raidus, top radius -->
              <DefaultValue>10 ,0.5, 0.5</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
        <Group Name="layer materials" Extensible="true" NumberOfRequiredGroups="1" AdvanceLevel="11">
          <BriefDescription>A user assigned a set of materials which form the inner layers of the nulcear pin</BriefDescription>
          <DetailedDescription>
             A user assigned a set of materials which form the inner layers of the nulcear pin.
          </DetailedDescription>
          <ItemDefinitions>
            <Int Name="sub material" NumberOfRequiredValues="1" AdvanceLevel="11">
              <BriefDescription>A user assigned material for a nulcear pin layer</BriefDescription>
              <DetailedDescription>
                A user assigned material for a nulcear pin layer
              </DetailedDescription>
              <DefaultValue>0</DefaultValue>
            </Int>
            <Double Name="radius(normalized)" NumberOfRequiredValues="1" AdvanceLevel="11">
              <DefaultValue>1</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create pin)" BaseType="result">
      <ItemDefinitions>
        <!-- The create pin is returned in the base result's "create" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
