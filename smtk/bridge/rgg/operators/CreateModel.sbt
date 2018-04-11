<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "CreateModel" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create model" Label="Model - Create" BaseType="operator">
      <BriefDescription>Create a RGG model.</BriefDescription>
      <DetailedDescription>
        Create a RGG model with a simple nuclear core. Users can next use the "edit core" operator
        to change other properties. Ex. z origin, duct thickness and height. Geometry type should
        be decided at creation time and cannot be modified later.
      </DetailedDescription>
      <ItemDefinitions>
        <Int Name="lattice size" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
          <!-- Since SMTK does not support more than one default value we make it one and extensible -->
          <DefaultValue>0</DefaultValue>
        </Int>
        <String Name="name" NumberOfValuesRequired="1">
          <BriefDescription>A user-assigned name for the core.</BriefDescription>
          <DetailedDescription>
            A user-assigned name for the model.
            The name need not be unique, but unique names are best.
            If not assigned, a machine-generated name will be assigned.
          </DetailedDescription>
          <DefaultValue>nuclearCore</DefaultValue>
        </String>
        <String Name="geometry type" Label="geometry type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <ChildrenDefinitions>
            <Double Name="duct thickness X" NumberOfRequiredValues="1">
              <BriefDescription>Thickness of the associated rectilinear duct along x axis</BriefDescription>
              <DetailedDescription>
                Thickness of the associated rectilinear duct along x axis.
              </DetailedDescription>
              <DefaultValue>10.0</DefaultValue>
            </Double>
            <Double Name="duct thickness Y" NumberOfRequiredValues="1">
              <BriefDescription>Thickness of the associated rectilinear assembly along y axis</BriefDescription>
              <DetailedDescription>
                Thickness of the associated rectilinear assembly along y axis.
              </DetailedDescription>
              <DefaultValue>10.0</DefaultValue>
            </Double>
            <Double Name="duct thickness" NumberOfRequiredValues="1">
              <BriefDescription>Thickness of the hex duct</BriefDescription>
              <DetailedDescription>
                Thickness of the hex duct(radius of the inscribed circle).
              </DetailedDescription>
              <DefaultValue>10.0</DefaultValue>
            </Double>
            <Double Name="height" NumberOfRequiredValues="1">
              <BriefDescription>Height of the nuclear core</BriefDescription>
              <DetailedDescription>
                Height of the nuclear core. It would be consumed by related ducts.
              </DetailedDescription>
              <DefaultValue>10.0</DefaultValue>
            </Double>
            <Double Name="z origin" NumberOfRequiredValues="1" AdvanceLevel="0">
              <BriefDescription>z origin of the nuclear core</BriefDescription>
              <DetailedDescription>
              </DetailedDescription>
                Z origin of the core. It would be consumed by related ducts.
              <DefaultValue>0.0</DefaultValue>
            </Double>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum= "Hex">Hex</Value>
              <Items>
                <Item>z origin</Item>
                <Item>height</Item>
                <Item>duct thickness</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum= "Rect">Rect</Value>
              <Items>
                <Item>z origin</Item>
                <Item>height</Item>
                <Item>duct thickness X</Item>
                <Item>duct thickness Y</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
        <!-- The group item is just a place holder. -->
        <Group Name="assemblies and layouts" Extensible="true" NumberOfRequiredGroups="0" AdvanceLevel="11">
          <BriefDescription>A user assigned a set of assemblies which are laid out in the lattice</BriefDescription>
          <DetailedDescription>
            A user assigned a set of assemblies which are laid out in the lattice.
          </DetailedDescription>
          <ItemDefinitions>
            <String Name="assembly UUID" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
            </String>
            <Int Name="schema plan" NumberOfRequiredValues="2" Extensible="true" AdvanceLevel="11">
              <!-- Rect: (i, j) where i is the index along width and y is along height. Hex(i, j) where i is the index along the ring and j is the index on that layer -->
            </Int>
            <Double Name="coordinates" NumberOfRequiredValues="3" Extensible="true" AdvanceLevel="11">
              <!-- x, y and z coordinates -->
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create model)" BaseType="result">
      <ItemDefinitions>
        <!-- The created model is returned in the base result's "created" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
