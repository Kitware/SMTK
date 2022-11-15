<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6"  DisplayHint="true">
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <AttDef Type="BoundaryCondition" Label="BoundaryCondition" >
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
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Material" Label="Material">
      <ItemDefinitions>
        <Double Name="Density" Label="Density" >
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
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
