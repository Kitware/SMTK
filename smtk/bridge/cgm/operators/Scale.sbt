<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Scale" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="scale" BaseType="operator">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Apply a scale transform to a model.
      </BriefDescription>
      <DetailedDescription>
        Apply a uniform or per-axis scale transform to a model.
      </DetailedDescription>
      <ItemDefinitions>
        <Int Name="scale factor type">
          <ChildrenDefinitions>
            <!-- Option 1: width, depth, height -->
            <Double Name="scale factor" NumberOfRequiredValues="1">
              <BriefDescription>Uniform scale factor.</BriefDescription>
              <Min Inclusive="false">0</Min>
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <!-- Option 2: center, axis0, axis1, axis2, extension -->
            <Double Name="scale factors" NumberOfRequiredValues="3">
              <BriefDescription>Scale factor along each axis.</BriefDescription>
              <Min Inclusive="false">0</Min>
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
                <Label>Z</Label>
              </ComponentLabels>
              <DefaultValue>1.0</DefaultValue>
            </Double>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="uniform">0</Value>
              <Items>
                <Item>factor</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="per axis">1</Value>
              <Items>
                <Item>factors</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
        <Double Name="origin" NumberOfRequiredValues="3">
          <BriefDescription>Point about which to scale.</BriefDescription>
          <ComponentLabels>
            <Label>X</Label>
            <Label>Y</Label>
            <Label>Z</Label>
          </ComponentLabels>
          <DefaultValue>0.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(scale)" BaseType="result">
      <ItemDefinitions>
        <!-- The scaled model. -->
        <ModelEntity Name="bodies" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
