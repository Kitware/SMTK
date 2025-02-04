<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="createUniformGrid" Label="Regular Grid" BaseType="operation">
      <BriefDescription>
        Construct a simple uniform grid
      </BriefDescription>
      <ItemDefinitions>

        <String Name="dimension" Label="Dimension">

          <ChildrenDefinitions>

            <Double Name="origin2d" Label="Origin" NumberOfRequiredValues="2">
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
              </ComponentLabels>
              <DefaultValue>0,0</DefaultValue>
            </Double>

            <Double Name="size2d" Label="Size" NumberOfRequiredValues="2">
              <ComponentLabels>
                <Label>Length</Label>
                <Label>Width</Label>
              </ComponentLabels>
              <RangeInfo><Min Inclusive="false">0</Min></RangeInfo>
              <DefaultValue>1,1</DefaultValue>
            </Double>

            <Int Name="discretization2d" Label="Discretization" NumberOfRequiredValues="2">
              <ComponentLabels>
                <Label>Length</Label>
                <Label>Width</Label>
              </ComponentLabels>
              <RangeInfo><Min Inclusive="false">0</Min></RangeInfo>
              <DefaultValue>5,5</DefaultValue>
            </Int>

            <Double Name="origin3d" Label="Origin" NumberOfRequiredValues="3">
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
                <Label>Z</Label>
              </ComponentLabels>
              <DefaultValue>0,0,0</DefaultValue>
            </Double>

            <Double Name="size3d" Label="Size" NumberOfRequiredValues="3">
              <ComponentLabels>
                <Label>Length</Label>
                <Label>Width</Label>
                <Label>Height</Label>
              </ComponentLabels>
              <RangeInfo><Min Inclusive="false">0</Min></RangeInfo>
              <DefaultValue>1,1,1</DefaultValue>
            </Double>

            <Int Name="discretization3d" Label="Discretization" NumberOfRequiredValues="3">
              <ComponentLabels>
                <Label>Length</Label>
                <Label>Width</Label>
                <Label>Height</Label>
              </ComponentLabels>
              <RangeInfo><Min Inclusive="false">0</Min></RangeInfo>
              <DefaultValue>5,5,5</DefaultValue>
            </Int>

          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="2">2</Value>
              <Items>
                <Item>origin2d</Item>
                <Item>size2d</Item>
                <Item>discretization2d</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="3">3</Value>
              <Items>
                <Item>origin3d</Item>
                <Item>size3d</Item>
                <Item>discretization3d</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <Resource Name="resource" Label="Import into" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <Accepts>
            <Resource Name="smtk::session::mesh::Resource"/>
          </Accepts>
        </Resource>

        <String Name="session only" Label="session" AdvanceLevel="1">
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="this file">import into this file </Value>
            </Structure>
            <Structure>
              <Value Enum="this session">import into a new file using this file's session</Value>
            </Structure>
          </DiscreteInfo>
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(createBackgroundDomain)" BaseType="result"/>
  </Definitions>

  <Views>
    <View Type="Operation" Title="Model - Create Uniform Grid"
          FilterByAdvanceLevel="true" UseSelectionManager="true">
      <InstancedAttributes>
        <Att Type="createUniformGrid"/>
      </InstancedAttributes>
    </View>
  </Views>

</SMTK_AttributeResource>
