<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "CreateEdge" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create edge" BaseType="operator">
      <ItemDefinitions>
        <Int Name="curve type" NumberOfRequiredValues="1">
          <BriefDescription>The type of curve to create.</BriefDescription>
          <ChildrenDefinitions>
            <Double Name="point" NumberOfRequiredValues="3">
              <BriefDescription>The (x,y,z) coordinates of an intermediate point.</BriefDescription>
            </Double>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <!-- Values from CGM's GeometryType enum in util/GeometryDefines.h -->
            <Structure>
              <Value Enum="line segment">6</Value>
              <Items>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="arc">1</Value>
              <Items>
                <Item>point</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="ellipse">2</Value>
              <Items>
                <Item>point</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="parabola">3</Value>
              <Items>
                <Item>point</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="hyperbola">9</Value>
              <Items>
                <Item>point</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
        <ModelEntity Name="vertices" NumberOfRequiredValues="2">
          <BriefDescription>Two pre-existing model vertices.</BriefDescription>
        </ModelEntity>
        <Int Name="color" NumberOfRequiredValues="1">
          <BriefDescription>The CGM color index assigned to the edge.</BriefDescription>
          <Min Inclusive="true">0</Min>
          <Max Inclusive="true">15</Max>
          <DefaultValue>1</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create edge)" BaseType="result">
      <ItemDefinitions>
        <!-- The edge created. -->
        <ModelEntity Name="edge" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
