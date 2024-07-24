<?xml version="1.0"?>
<!--Created by XmlV4StringWriter-->
<SMTK_AttributeResource Version="8"  DisplayHint="true">
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="doubleFunc" Association="None" Units="m"/>
    <AttDef Type="A" Label="A" BaseType="" Unique="false" Units="m">
      <ItemDefinitions>
        <Double Name="d0" Label="Optional Double" Optional="true">
        </Double>
        <Double Name="d1" Label="Expression Double">
          <ExpressionType>doubleFunc</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>
   </Definitions>
  <!--**********  Attribute Instances ***********-->
  <Views>
    <View Type="Instanced" Title="DoubleItemTest" Label="Simple Double Item Test" TopLevel="true">
      <InstancedAttributes>
        <Att Type="A" Name="Attribute (editable units)">
          <ItemViews>
            <View Item="d1" ExpressionOnly="true"/>
          </ItemViews>
        </Att>
        <Att Type="A" Name="Attribute (read only units)" UnitsMode="ViewOnly">
          <ItemViews>
            <View Item="d1" ExpressionOnly="true"/>
          </ItemViews>
        </Att>
        <Att Type="A" Name="Attribute (don't show units)" UnitsMode="None">
          <ItemViews>
            <View Item="d1" ExpressionOnly="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
