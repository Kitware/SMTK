<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "CreateBrick" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create brick" BaseType="operator">
      <BriefDescription>
        Create a parallelepiped or rhombus.
      </BriefDescription>
      <DetailedDescription>
        Create a cuboid by specifying width, depth, and height;
        or for the more general case, a parallelepiped or rhombus
        by specifying a center point;
        either 2 or 3 axes (they need not be orthogonal); and
        an "extension" along each axis from the center point.
      </DetailedDescription>
      <ItemDefinitions>
        <Int Name="construction method">
          <ChildrenDefinitions>
            <!-- Option 1: width, depth, height -->
            <Double Name="width" NumberOfRequiredValues="1">
              <Min Inclusive="false">0</Min>
              <DefaultValue>1.0</DefaultValue>
              <BriefDescription>Width of the cuboid (along x axis).</BriefDescription>
            </Double>
            <Double Name="depth" NumberOfRequiredValues="1">
              <Min Inclusive="false">0</Min>
              <DefaultValue>1.0</DefaultValue>
              <BriefDescription>Depth of the cuboid (along y axis).</BriefDescription>
            </Double>
            <Double Name="height" NumberOfRequiredValues="1">
              <Min Inclusive="false">0</Min>
              <DefaultValue>1.0</DefaultValue>
              <BriefDescription>Height of the cuboid (along z axis).</BriefDescription>
            </Double>
            <!-- Option 2: center, axis0, axis1, axis2, extension -->
            <Double Name="center" NumberOfRequiredValues="3">
              <DefaultValue>0.0</DefaultValue>
              <BriefDescription>Center of the brick.</BriefDescription>
            </Double>
            <Double Name="axis 0" NumberOfRequiredValues="3">
              <BriefDescription>Direction of a principal brick axis.</BriefDescription>
              <DefaultValue>1,0,0</DefaultValue>
            </Double>
            <Double Name="axis 1" NumberOfRequiredValues="3">
              <BriefDescription>Direction of a principal brick axis.</BriefDescription>
              <DefaultValue>0,1,0</DefaultValue>
            </Double>
            <Double Name="axis 2" NumberOfRequiredValues="3">
              <BriefDescription>Direction of a principal brick axis.</BriefDescription>
              <DefaultValue>0,0,1</DefaultValue>
            </Double>
            <Double Name="extension" NumberOfRequiredValues="3">
              <Min Inclusive="true">0</Min>
              <DefaultValue>0.5</DefaultValue>
              <BriefDescription>Distance between center point and face along the respective principal axis.</BriefDescription>
            </Double>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
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
      <!-- The brick created is stored in the base result's "created" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
