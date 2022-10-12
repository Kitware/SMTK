<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6"  NameSeparator="-" DisplayHint="true">
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="BodyInfo" Label="BodyInfo">
      <ItemDefinitions>
        <Component Name="material" Label="material" Version="0" EnforceCategories="false" Role="-2" NumberOfRequiredValues="1">
          <Accepts>
            <Resource Name="smtk::attribute::Resource" Filter="attribute[type='Material']" />
          </Accepts>
          <Rejects />
          <ChildrenDefinitions>
            <Double Name="initialFlow" NumberOfRequiredValues="3"/>
            <Double Name="initialTemp" NumberOfRequiredValues="1"/>
          </ChildrenDefinitions>
          <ConditionalInfo>
            <Condition Resource="smtk::attribute::Resource" Component="attribute[type='SolidMaterial']">
              <Items>
                <Item>initialTemp</Item>
              </Items>
            </Condition>
            <Condition Component="attribute[type='LiquidMaterial']">
              <Items>
                <Item>initialTemp</Item>
                <Item>initialFlow</Item>
              </Items>
            </Condition>
          </ConditionalInfo>
        </Component>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Material" Abstract="true"/>
    <AttDef Type="LiquidMaterial" BaseType="Material" />
    <AttDef Type="SolidMaterial" BaseType="Material" />
    <AttDef Type="VoidMaterial" BaseType="Material"/>
  </Definitions>
 <Views>
    <View Type="Group" Name="TopLevel" FilterByAdvanceLevel="true" TabPosition="South" TopLevel="true">
      <Views>
        <View Title="Materials" />
        <View Title="BodyInfos" />
      </Views>
    </View>
    <View Type="Attribute" Title="Materials">
      <AttributeTypes>
        <Att Type="Material"/>
      </AttributeTypes>
    </View>
    <View Type="Attribute" Title="BodyInfos" Label="Body Information">
      <AttributeTypes>
        <Att Type="BodyInfo"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
