<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <AttDef Type="Test" Label="Test" BaseType="" >
      <ItemDefinitions>
        <Double Name="a" Label="Should See this" >
        </Double>
        <Double Name="b" Label="Should NOT See this" >
          <DefaultValue> 10 </DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Instanced" Title="Test" Label="Null Test" TopLevel="true">
      <InstancedAttributes>
        <Att Type="Test" Name="test">
          <ItemViews>
            <View Item="b" Type="null"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
