<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Model "ModelGroup" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="entity group" Label="Model - Create Group" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <String Name="Operation" Label="Operation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>operation for the operator</BriefDescription>
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
              <BriefDescription>Group type for the discrete model kernel:
          Boundary group (face or edge) is not partitioned, meaning each entity can belong to multiple boundary groups;
          Domain group (volume or face) is partitioned, meaning each entity will only belong to one domain group.
              </BriefDescription>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Cover">0</Value>
                <Value Enum="Partition">1</Value>
              </DiscreteInfo>
            </Int>
            <Void Name="Vertex" Label="Vertex" Version="0" NumberOfRequiredValues="1" Optional="true" AdvanceLevel = "1" Option = "true" IsEnabledByDefault = "true">
              <BriefDescription>Allow vertex to be added to the group.
              </BriefDescription> 
            </Void>
            <Void Name="Edge" Label="Edge" Version="0" NumberOfRequiredValues="1" Optional="true" AdvanceLevel = "1" Option = "true" IsEnabledByDefault = "true">
              <BriefDescription>Allow edge to be added to the group.
              </BriefDescription>  
            </Void>
            <Void Name="Face" Label="Face" Version="0" NumberOfRequiredValues="1" Optional="true" AdvanceLevel = "1" Option = "true" IsEnabledByDefault = "true"> 
              <BriefDescription>Allow face to be added to the group.
              </BriefDescription> 
            </Void>
            <Void Name="Volume" Label="Volume" Version="0" NumberOfRequiredValues="1" Optional="true" AdvanceLevel = "1" Option = "true" IsEnabledByDefault = "true">
              <BriefDescription>Allow volume to be added to the group.
              </BriefDescription>  
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
