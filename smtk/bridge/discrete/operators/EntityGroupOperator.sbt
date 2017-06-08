<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "ModelGroup" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="entity group" BaseType="operator">
      <BriefDescription>
        Create a group of cell entities. User can modify and remove the created group afterwards.
      </BriefDescription>
      <DetailedDescription>
        Create a group of cell entities. User can modify and remove the created group afterwards.

        Boundary groups are intended to hold cell entities of the highest dimension that the model contains.
        Domain groups are intended to hold cell entities that are lower than the highest dimension
        that the mode contains.
      </DetailedDescription>
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <String Name="Operation" Label="Operation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>
            The operation determines which action to take on the group: create it, modify its membership, or remove it.
          </BriefDescription>
          <DetailedDescription>
            The operation determines which action to take on the group: create it, modify its membership, or remove it.
          </DetailedDescription>
          <ChildrenDefinitions>
            <ModelEntity Name="modify cell group" NumberOfRequiredValues="1">
              <MembershipMask>group</MembershipMask>
            </ModelEntity>
            <ModelEntity Name="remove cell group" Extensible="1" NumberOfRequiredValues="0">
              <MembershipMask>group</MembershipMask>
            </ModelEntity>
            <ModelEntity Name="cell to add" NumberOfRequiredValues="0" Extensible="1">
              <MembershipMask>volume|face|edge</MembershipMask>
            </ModelEntity>
            <ModelEntity Name="cell to remove" NumberOfRequiredValues="0" Extensible="1">
              <MembershipMask>volume|face|edge</MembershipMask>
            </ModelEntity>
            <Int Name="group type" Label="Group Type:" Version="0" NumberOfRequiredValues="1">
              <BriefDescription>
                Group type for the discrete model kernel:

                Boundary groups are intended to hold cell entities of the highest dimension that the model contains.
                Domain groups are intended to hold cell entities that are lower than the highest dimension
                that the mode contains.
              </BriefDescription>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Boundary">0</Value>
                <Value Enum="Domain">1</Value>
              </DiscreteInfo>
            </Int>
            <String Name="group name" Label="group name" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>new group</DefaultValue>
            </String>
          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Create Group">Create</Value>
              <Items>
                <Item>group type</Item>
                <Item>group name</Item>
                <Item>cell to add</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Remove Group">Remove</Value>
              <Items>
                <Item>remove cell group</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Modify Group">Modify</Value>
              <Items>
                <Item>modify cell group</Item>
                <Item>cell to add</Item>
                <Item>cell to remove</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <AttDef Type="result(entity group)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
