<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "EditAssembly" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit assembly" Label="Model - Edit Assembly" BaseType="operator">
      <BriefDescription>Edit a RGG Assembly.</BriefDescription>
      <DetailedDescription>
        User can eidt an existing assembly by changing its properties. A schema planner
        is provided to plan the layout of nuclear pins in the assembly.
      </DetailedDescription>
      <AssociationsDef Name="assembly" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>group</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned name for the nulcear assembly</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nulcear assembly.
          </DetailedDescription>
        </String>
        <String Name="label" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned label for the nulcear assembly</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nulcear assembly.
          </DetailedDescription>
        </String>
        <Group Name="pins and layouts" Extensible="true" NumberOfRequiredGroups="0" AdvanceLevel="11">
          <BriefDescription>A user assigned a set of pins which are laid out in the lattice</BriefDescription>
          <DetailedDescription>
            A user assigned a set of pins which are laid out in the lattice.
          </DetailedDescription>
          <ItemDefinitions>
            <String Name="pin UUID" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
            </String>
            <Int Name="schema plan" NumberOfRequiredValues="2" Extensible="true" AdvanceLevel="11">
              <!-- Rect: (i, j) where i is the index along width and y is along height. Hex(i, j) where i is the index along the ring and j is the index on that layer -->
            </Int>
            <Double Name="coordinates" NumberOfRequiredValues="2" Extensible="true" AdvanceLevel="11">
              <!-- x, y and z coordinates -->
            </Double>
          </ItemDefinitions>
        </Group>
        <ModelEntity Name="associated duct" NumberOfRequiredValues="1" AdvanceLevel="0">
          <MembershipMask>aux_geom</MembershipMask>
          <BriefDescription>A user assigned duct which bounds the pins in the assembly</BriefDescription>
          <DetailedDescription>
            A user assigned duct which bounds the pins in the assembly. It would define the size
            of the assembly.
          </DetailedDescription>
        </ModelEntity>
        <ModelEntity Name="instance to be deleted" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11">
          <MembershipMask>instance</MembershipMask>
          <BriefDescription>instances which should be deleted from current assembly</BriefDescription>
          <DetailedDescription>
            Instances which should be deleted from current assembly.
          </DetailedDescription>
        </ModelEntity>
        <ModelEntity Name="instance to be added" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11">
          <MembershipMask>instance</MembershipMask>
          <BriefDescription>instances which should be added into current assembly</BriefDescription>
          <DetailedDescription>
            Instances which should be added into current assembly.
          </DetailedDescription>
        </ModelEntity>
        <Void Name="center pins" NumberOfRequiredValues="1" Optional="true" IsEnabledByDafault="true" AdvanceLevel="11">
        </Void>
        <Double Name="pitches" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
          <BriefDescription>distance between two adjacent pin centers</BriefDescription>
          <DetailedDescription>
            Distance between two adjacent pin centers. For now it's calculated by a formula provided by the vendor.
          </DetailedDescription>
          <!-- Since SMTK does not support more than one default values we make it one and extensible -->
        </Double>
        <Int Name="lattice size" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
          <!-- Since SMTK does not support more than one default value we make it one and extensible -->
        </Int>
        <Int Name="z axis" NumberOfRequiredValues="1" AdvanceLevel="11">
        </Int>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(edit assembly)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0" Extensible="true"/>
        <Void Name="force camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkRGGEditAssemblyView ...)
      -->
    <View Type="smtkRGGEditAssemblyView" Title="Edit Assembly"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="false">
      <Description>
        Change a nulcear assembly's properties and layout.
      </Description>
      <AttributeTypes>
        <Att Type="edit assembly"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
