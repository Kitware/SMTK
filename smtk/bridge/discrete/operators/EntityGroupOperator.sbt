<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "ModelGroup" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="entity group" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <String Name="Operation" Label="Operation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>operation for the operator</BriefDescription>
          <ChildrenDefinitions>
            <ModelEntity Name="modify cell group" NumberOfRequiredValues="1">
              <!-- There seems to be a bug in checking the validity of the entity being set 
              when the membership is group. Skip for now 
              <MembershipMask>group</MembershipMask>  -->
            </ModelEntity>
            <ModelEntity Name="remove cell group" Extensible="1" NumberOfRequiredValues="1">
              <!-- There seems to be a bug in checking the validity of the entity being set 
              when the membership is group. Skip for now 
              <MembershipMask>group</MembershipMask>  -->
            </ModelEntity>
            <ModelEntity Name="cell to add" Extensible="1">
              <MembershipMask>face|edge|vertex</MembershipMask>
            </ModelEntity>
            <ModelEntity Name="cell to remove" Extensible="1">
              <MembershipMask>face|edge|vertex</MembershipMask>
            </ModelEntity>
            <Int Name="entity type" Label="Entity Type:" Version="0" NumberOfRequiredValues="1">
              <BriefDescription>EnityType that new built group will contain</BriefDescription>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Face">0</Value>
                <Value Enum="Edge">1</Value>
                <Value Enum="Vertex">2</Value>
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
                <Item>group name</Item>
                <Item>entity type</Item>
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
        <ModelEntity Name="new entities" NumberOfRequiredValues="0" Extensible="1">
        </ModelEntity>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
