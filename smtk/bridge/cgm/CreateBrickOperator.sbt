<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "CreateBrick" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create brick" BaseType="operator">
      <ItemDefinitions>
        <Int Name="construction method">
          <ChildrenDefinitions>
            <!-- Option 1: width, depth, height -->
            <Double Name="width" NumberOfRequiredValues="1">
              <Min Inclusive="false">0</Min>
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <Double Name="depth" NumberOfRequiredValues="1">
              <Min Inclusive="false">0</Min>
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <Double Name="height" NumberOfRequiredValues="1">
              <Min Inclusive="false">0</Min>
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <!-- Option 2: center, axis0, axis1, axis2, extension -->
            <Double Name="center" NumberOfRequiredValues="3">
              <DefaultValue>0.0</DefaultValue>
            </Double>
            <Double Name="axis 0" NumberOfRequiredValues="3"/>
            <Double Name="axis 1" NumberOfRequiredValues="3"/>
            <Double Name="axis 2" NumberOfRequiredValues="3"/>
            <Double Name="extension" NumberOfRequiredValues="3">
              <Min Inclusive="true">0</Min>
              <DefaultValue>0.5</DefaultValue>
            </Double>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="1">
            <Structure>
              <Value Enum="axis-aligned">0</Value>
              <Items>
                <Item>width</Item>
                <Item>depth</Item>
                <Item>height</Item>
                <Item>center</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="parallepiped">0</Value>
              <Items>
                <Item>center</Item>
                <Item>axis 0</Item>
                <Item>axis 1</Item>
                <Item>axis 2</Item>
                <Item>extension</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create brick)" BaseType="result">
      <ItemDefinitions>
        <!-- The brick created. -->
        <ModelEntity Name="bodies" NumberOfRequiredValues="1" Extensible="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
