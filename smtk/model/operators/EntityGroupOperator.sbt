<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Model "ModelGroup" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="entity group" Label="Model - Create Group" BaseType="operator">
      <BriefDescription>
        Create a group of cell entities. User can modify and remove the created group afterwards.
      </BriefDescription>
      <DetailedDescription>
        Create a group of cell entities. User can modify and remove the created group afterwards.

        If advance level is turned on, user can filter the entities by their cell type then use them to modify the group.
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
            <Void Name="Vertex" Label="Vertex" Version="0" NumberOfRequiredValues="1" Optional="true" AdvanceLevel = "1" Option = "true" IsEnabledByDefault = "true">
              <BriefDescription>Allow vertices to be added to the group.</BriefDescription>
              <DetailedDescription>Allow vertices to be added to the group.</DetailedDescription>
            </Void>
            <Void Name="Edge" Label="Edge" Version="0" NumberOfRequiredValues="1" Optional="true" AdvanceLevel = "1" Option = "true" IsEnabledByDefault = "true">
              <BriefDescription>Allow edges to be added to the group.</BriefDescription>
              <DetailedDescription>Allow edges to be added to the group.</DetailedDescription>
            </Void>
            <Void Name="Face" Label="Face" Version="0" NumberOfRequiredValues="1" Optional="true" AdvanceLevel = "1" Option = "true" IsEnabledByDefault = "true"> 
              <BriefDescription>Allow faces to be added to the group.</BriefDescription>
              <DetailedDescription>Allow faces to be added to the group.</DetailedDescription>
            </Void>
            <Void Name="Volume" Label="Volume" Version="0" NumberOfRequiredValues="1" Optional="true" AdvanceLevel = "1" Option = "true" IsEnabledByDefault = "true">
              <BriefDescription>Allow volumes to be added to the group.</BriefDescription>
              <DetailedDescription>Allow volumes to be added to the group.</DetailedDescription>
            </Void>
            <String Name="group name" Label="group name" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>new group</DefaultValue>
            </String>
          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Create Group">Create</Value>
              <Items>
                <Item>group name</Item>
                <Item>cell to add</Item>
                <Item>Vertex</Item>
                <Item>Edge</Item>
                <Item>Face</Item>
                <Item>Volume</Item>
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
                <Item>Vertex</Item>
                <Item>Edge</Item>
                <Item>Face</Item>
                <Item>Volume</Item>
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

