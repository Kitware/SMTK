<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6"  DisplayHint="true">
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <AttDef Type="BoundaryCondition" Label="BoundaryCondition" Abstract="true">
      <ItemDefinitions>
        <String Name="Note">
      </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Pressure" Label="Pressure" BaseType="BoundaryCondition">
      <ItemDefinitions>
        <Double Name="Pressure" Label="Pressure" >
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Velocity" Label="Velocity" BaseType="BoundaryCondition">
      <ItemDefinitions>
        <Double Name="Velocity" Label="Velocity" NumberOfRequiredValues="3">
          <ComponentLabels>
            <Label>x=</Label>
            <Label>y=</Label>
            <Label>z=</Label>
          </ComponentLabels>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Material" Label="Material">
      <ItemDefinitions>
        <String Name="Density">
          <ChildrenDefinitions>
            <Double Name="value" Label="Value">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="temp" Label="Reference Temp">
            </Double>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>Normal</Value>
              <Items>
                <Item>value</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>Boussinesq</Value>
              <Items>
                <Item>value</Item>
                <Item>temp</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <Double Name="Viscosity" Label="Viscosity">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

 <Views>
    <View Type="Group" Name="TopLevel" FilterByAdvanceLevel="true" TabPosition="North" TopLevel="true">
      <Views>
        <View Title="Materials" />
        <View Title="Boundary Conditions" />
      </Views>
    </View>
    <View Type="Attribute" Title="Materials">
      <AttributeTypes>
        <Att Type="Material"/>
      </AttributeTypes>
    </View>
    <View Type="Attribute" Title="Boundary Conditions">
      <AttributeTypes>
        <Att Type="BoundaryCondition"/>
      </AttributeTypes>
    </View>
  </Views>

</SMTK_AttributeResource>
