<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6" ID="a74eb19f-4ca0-4228-995b-6086e37f4b1e" NameSeparator="-" DisplayHint="true">
  <ActiveCategories Enabled="false" />
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="A" Label="A" Unique="true">
      <AssociationsDef Name="AAssociations" Label="AAssociations" Version="0" EnforceCategories="false" Role="-1" NumberOfRequiredValues="0" Extensible="true">
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="face" />
        </Accepts>
        <Rejects />
      </AssociationsDef>
    </AttDef>
    <AttDef Type="A1" Label="A1" BaseType="A" >
     </AttDef>
    <AttDef Type="B" Label="B" >
      <AssociationsDef Name="BAssociations" Label="BAssociations" Version="0" EnforceCategories="false" Role="-1" NumberOfRequiredValues="0" Extensible="true">
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="face" />
        </Accepts>
        <Rejects />
      </AssociationsDef>
    </AttDef>
    <AttDef Type="C" Label="C" >
      <AssociationsDef Name="CAssociations" Label="CAssociations" Version="0" EnforceCategories="false" Role="-1" NumberOfRequiredValues="0" Extensible="true">
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="face" />
        </Accepts>
        <Rejects />
      </AssociationsDef>
    </AttDef>
  </Definitions>
  <Exclusions>
    <Rule>
      <Def>A</Def>
      <Def>B</Def>
    </Rule>
    <Rule>
      <Def>B</Def>
      <Def>C</Def>
    </Rule>
  </Exclusions>
  <Prerequisites>
    <Rule Type="A">
      <Def>C</Def>
    </Rule>
  </Prerequisites>
 <Views>
    <View Type="Attribute" Title="Attributes" TopLevel="true">
      <AttributeTypes>
        <Att Type="A"/>
        <Att Type="B"/>
        <Att Type="C"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
