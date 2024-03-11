<?xml version="1.0"?>
<!--Created by XmlV4StringWriter-->
<SMTK_AttributeResource Version="4">
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="doubleFunc" Association="None"/>
    <AttDef Type="A" Label="A" BaseType="" Unique="false">
      <AssociationsDef Name="simple" Label="simple" NumberOfRequiredValues="0" Extensible="true">
        <Accepts>
          <Resource Name="smtk::markup::Resource" Filter="smtk::markup::UnstructuredData" />
        </Accepts>
      </AssociationsDef>      <ItemDefinitions>
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
    <View Type="Instanced" Title="AttributeResourceCloneTest" Label="Simple Resource Clone Test" TopLevel="true">
      <InstancedAttributes>
        <Att Type="A" Name="Test Attribute">
          <ItemViews>
            <View Item="d1" ExpressionOnly="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
