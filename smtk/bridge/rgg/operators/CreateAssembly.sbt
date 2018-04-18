<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "CreateAssembly" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create assembly" Label="Model - Create Assembly" BaseType="operator">
      <BriefDescription>Create a RGG Assembly.</BriefDescription>
      <DetailedDescription>
        By providing a name user can create a simple empty assembly.
        After the creation, CMB would automatically switch to "Edit Assembly" operator
        so that user can tweak other properties.
        In SMTK world, assembly's type is group.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="0">
          <BriefDescription>A user assigned name for the nuclear assembly</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nuclear assembly.
          </DetailedDescription>
          <DefaultValue>Assembly0</DefaultValue>
        </String>
        <String Name="label" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned label for the nuclear assembly</BriefDescription>
          <DetailedDescription>
            A user assigned label for the nuclear assembly.
          </DetailedDescription>
          <DefaultValue>A0</DefaultValue>
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
            <Double Name="coordinates" NumberOfRequiredValues="3" Extensible="true" AdvanceLevel="11">
              <!-- x, y and z coordinates -->
            </Double>
          </ItemDefinitions>
        </Group>
        <ModelEntity Name="associated duct" NumberOfRequiredValues="0" AdvanceLevel="11">
          <BriefDescription>A user assigned duct which bounds the pins in the assembly</BriefDescription>
          <DetailedDescription>
            A user assigned duct which bounds the pins in the assembly. It's just a place holder in create mode.
          </DetailedDescription>
        </ModelEntity>
        <Void Name="center pins" NumberOfRequiredValues="1" Optional="true" IsEnabledByDafault="true" AdvanceLevel="11">
        </Void>
        <Double Name="pitches" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
          <BriefDescription>distance between two adjacent pins</BriefDescription>
          <!-- Since SMTK does not support more than one default values we make it one and extensible -->
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <Int Name="lattice size" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
          <!-- Since SMTK does not support more than one default value we make it one and extensible -->
          <DefaultValue>0</DefaultValue>
        </Int>
        <Int Name="z axis" NumberOfRequiredValues="1" AdvanceLevel="11">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create assembly)" BaseType="result">
      <ItemDefinitions>
        <!-- The created assembly is returned in the base result's "create" item. -->
        <Void Name="force camera reset" Optional="true" IsEnabledByDefault="true" AdvanceLevel="11"/>
        <Void Name="hide other entities" Optional="true" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
