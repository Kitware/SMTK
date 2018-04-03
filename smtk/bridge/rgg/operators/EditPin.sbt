<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "EditPin" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit pin" Label="Model - Edit Pin" BaseType="operator">
      <BriefDescription>Edit a RGG Pin.</BriefDescription>
      <DetailedDescription>
        Edit an existing RGG Pin. The pin may consists of several cylinders and
        frustums. It can also have several materals defined from the out layer
        to inner layer.
      </DetailedDescription>
      <AssociationsDef Name="pin" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>aux_geom</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned name for the nuclear pin</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nuclear pin.
          </DetailedDescription>
        </String>
        <String Name="label" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned label for the nuclear pin</BriefDescription>
          <DetailedDescription>
            A user assigned label for the nuclear pin.
          </DetailedDescription>
        </String>
        <Int Name="cell material" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned outer material for the nuclear pin</BriefDescription>
          <DetailedDescription>
            A user assigned outer material for the nuclear pin.
          </DetailedDescription>
        </Int>
        <Void Name="cut away" NumberOfRequiredValues="1" Optional = "true" IsEnabledByDafault="false" AdvanceLevel="11">
          <BriefDescription>cut away the pin so that the inner layers are visible</BriefDescription>
          <DetailedDescription>
            If enabled, SMTK will use a clipping plane that is
            perpendicular to the bottom and goes through the base
            radius to cut the pin. By doing so, the user is able to
            view the inner layers.
          </DetailedDescription>
        </Void>
        <Double Name="z origin" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned z origin for the nuclear pin</BriefDescription>
          <DetailedDescription>
            A user assigned z origin for the nuclear pin.
          </DetailedDescription>
        </Double>
        <Group Name="pieces" Extensible="true" NumberOfRequiredGroups="1" AdvanceLevel="11">
          <BriefDescription>A user assigned a set of sections which form the the nuclear pin from bottom to top</BriefDescription>
          <DetailedDescription>
             A user assigned a set of sections which form the the nuclear pin from bottom to top
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
            </Double>
          </ItemDefinitions>
        </Group>
        <Group Name="layer materials" Extensible="true" NumberOfRequiredGroups="1" AdvanceLevel="11">
          <BriefDescription>A user assigned a set of materials which form the inner layers of the nuclear pin</BriefDescription>
          <DetailedDescription>
             A user assigned a set of materials which form the inner layers of the nuclear pin.
          </DetailedDescription>
          <ItemDefinitions>
            <Int Name="sub material" NumberOfRequiredValues="1" AdvanceLevel="11">
              <BriefDescription>A user assigned material for a nuclear pin layer</BriefDescription>
              <DetailedDescription>
                A user assigned material for a nuclear pin layer
              </DetailedDescription>
            </Int>
            <Double Name="radius(normalized)" NumberOfRequiredValues="1" AdvanceLevel="11">
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(edit pin)" BaseType="result">
      <ItemDefinitions>
        <!-- The edited pin is returned in the base result's "edit" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkRGGEditPinView ...)
      -->
    <View Type="smtkRGGEditPinView" Title="Edit Din"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="false">
      <Description>
        TODO: Add documentation for edit pin operator.
      </Description>
      <AttributeTypes>
        <Att Type="edit pin"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
