<?xml version="1.0"?>
<!--Created by XmlV4StringWriter-->
<SMTK_AttributeResource Version="8"  DisplayHint="true">
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="A" Label="A" BaseType="" Unique="false">
      <ItemDefinitions>
        <String Name="s" Label="MultiLine String" MultipleLines="true"/>
        <String Name="s1" Label="MultiLine String Fixed" MultipleLines="true"/>
      </ItemDefinitions>
    </AttDef>
   </Definitions>
  <!--**********  Attribute Instances ***********-->
  <Views>
    <View Type="Instanced" Title="StringItemTest" Label="Simple String Item Test" TopLevel="true">
      <InstancedAttributes>
        <Att Type="A" Name="Test Attribute">
          <ItemViews>
            <View Item="s" ExpandInY="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
