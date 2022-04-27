<?xml version="1.0"?>
  <!--
  This is a sample SBT file that demonstrates the accept and reject options in an attribute's
  associations.

  There are 2 general classes of Attributes (Suppliers and Consumers).  In addition to basic Supplier,
  there are 2 derived types Alpha and Beta.

  In the case of Consumers, the base type is abstract but there are 2 non-abstract derived types:

  1. NotAlphaConsumer - a Consumer that can not be associated with an type Alpha Supplier
  2. NotBeteConsumer - a Consumer that can not be associated with a type Beta Consumer

  Views have been defined to allow the user to create Suppliers and Consumers.
  The user is able to create Consumers and Suppliers and validate the above constraints when associating Suppliers to a Consumer.
  -->
<SMTK_AttributeResource Version="4">
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <AttDef Type="Supplier"/>
    <AttDef Type="Alpha" BaseType="Supplier" />
    <AttDef Type="Beta" BaseType="Supplier" />
    <AttDef Type="Consumer" Abstract="1" />
    <AttDef Type="NotBetaConsumer" BaseType="Consumer">
      <AssociationsDef Name="assoc" Extensible="true" NumberOfRequiredValues="0">
        <Accepts>
          <Resource Name="smtk::attribute::Resource" Filter="attribute[type='Supplier']"/>
        </Accepts>
        <Rejects>
          <Resource Name="smtk::attribute::Resource" Filter="attribute[type='Beta']"/>
        </Rejects>
      </AssociationsDef>
    </AttDef>
    <AttDef Type="NotAlphaConsumer" BaseType="Consumer">
      <AssociationsDef Name="assoc" Extensible="true" NumberOfRequiredValues="0">
        <Accepts>
          <Resource Name="smtk::attribute::Resource" Filter="attribute[type='Supplier']"/>
        </Accepts>
        <Rejects>
          <Resource Name="smtk::attribute::Resource" Filter="attribute[type='Alpha']"/>
        </Rejects>
      </AssociationsDef>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Group" Title="TopLevel" TopLevel="true" TabPosition="North">
      <Views>
        <View Title="Suppliers" />
        <View Title="Consumers" />
      </Views>
    </View>
    <View Type="Attribute" Title="Consumers" DisableTopButtons="false">
      <AttributeTypes>
        <Att Type="Consumer"/>
      </AttributeTypes>
    </View>
    <View Type="Attribute" Title="Suppliers"  DisableTopButtons="false">
      <AttributeTypes>
        <Att Type="Supplier"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
