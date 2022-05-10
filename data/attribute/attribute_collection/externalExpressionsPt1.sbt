<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <AttDef Type="A" Label="Consumer" BaseType="" >
      <ItemDefinitions>
        <Double Name="foo">
          <DefaultValue>0.0</DefaultValue>
          <ExpressionType>B-expressions</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Attribute" Title="External Expression Test Pt - Consumer" TopLevel="true" DisableTopButtons="false">
      <AttributeTypes>
        <Att Type="A"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
