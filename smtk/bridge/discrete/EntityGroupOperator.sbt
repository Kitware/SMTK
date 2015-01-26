<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "ModelEntityGroup" Operator -->
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
            <ModelEntity Name="cell group" NumberOfRequiredValues="1">
              <MembershipMask>group</MembershipMask>
            </ModelEntity>
            <ModelEntity Name="cell to add" NumberOfRequiredValues="1" Extensible="1">
              <MembershipMask>face|edge|vertex</MembershipMask>
            </ModelEntity>
            <ModelEntity Name="cell to remove" NumberOfRequiredValues="1" Extensible="1">
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

          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Create Group">Create</Value>
              <Items>
                <Item>entity type</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Remove Group">Remove</Value>
              <Items>
                <Item>cell group</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Modify Group">Modify</Value>
              <Items>
                <Item>cell group</Item>
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
