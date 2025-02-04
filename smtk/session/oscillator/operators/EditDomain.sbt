<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="editDomain" Label="Oscillator Model Domain" BaseType="operation">
      <BriefDescription>Edit a simulation domain that is a uniform grid</BriefDescription>
      <AssociationsDef Name="input" Label="Input"
                       NumberOfRequiredValues="0"
                       MaximumNumberOfValues="1" Extensible="true"
                       AdvanceLevel="1">
        <Accepts>
          <!-- We create a new volume if a model is provided or
               edit an existing volume if a volume is provided. -->
          <Resource Name="smtk::session::oscillator::Resource" Filter="model"/>
          <Resource Name="smtk::session::oscillator::Resource" Filter="volume"/>
        </Accepts>
      </AssociationsDef>

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

          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="1">
            <Structure>
              <Value Enum="2">2</Value>
              <Items>
                <Item>origin2d</Item>
                <Item>size2d</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="3">3</Value>
              <Items>
                <Item>origin3d</Item>
                <Item>size3d</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

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
    <AttDef Type="result(editDomain)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
