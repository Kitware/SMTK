
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Test">
      <AssociationsDef Name="assoc" Extensible="true" NumberOfRequiredValues="1">
        <Accepts>
          <Resource Name="smtk::attribute::Resource" Filter="attribute[type='Instance']"/>
        </Accepts>
      </AssociationsDef>
    </AttDef>
    <AttDef Type="Instance">
      <ItemDefinitions>
        <String Name="item">
          <DefaultValue>Some string</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Instance1">
      <ItemDefinitions>
        <String Name="item1">
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Group" Title="TopLevel" TopLevel="true" TabPosition="North">
      <Views>
        <View Title="TiledGroup"/>
        <View Title="Attribute"/>
      </Views>
    </View>
    <View Type="Group" Title="TiledGroup" TabPosition="North" Style="Tiled">
      <Views>
        <View Title="Instance"/>
        <View Title="Instance1"/>
      </Views>
    </View>
    <View Type="Instanced" Title="Instance">
      <InstancedAttributes>
        <Att Type="Instance" Name="Instance00"/>
      </InstancedAttributes>
    </View>
    <View Type="Instanced" Title="Instance1">
      <InstancedAttributes>
        <Att Type="Instance1" Name="Instance01"/>
      </InstancedAttributes>
    </View>
    <View Type="Attribute" Title="Attribute">
      <AttributeTypes>
        <Att Type="Test"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
