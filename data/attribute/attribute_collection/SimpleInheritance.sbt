<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="6">
  <Definitions>
    <AttDef Type="BaseDef">
      <ItemDefinitions>
        <String Name="Base String"/>
        <Int Name="Base Int"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="DerivedDef" BaseType="BaseDef">
      <ItemDefinitions>
        <Double Name="Derived Double"/>
       </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Attribute" Title="TopLevel" TopLevel="true" TabPosition="North">
      <AttributeTypes>
        <Att Type="BaseDef"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
