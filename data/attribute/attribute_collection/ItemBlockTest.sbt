<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">

  <Categories>
    <Cat>Solid Mechanics</Cat>
    <Cat>Fluid Flow</Cat>
    <Cat>Heat Transfer</Cat>
  </Categories>

  <ItemBlocks>
    <Block Name="B1">
      <ItemDefinitions>
        <String Name="s1">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i1"/>
      </ItemDefinitions>
    </Block>
  </ItemBlocks>

  <Definitions>
    <AttDef Type="Type1">
      <Categories>
        <Cat>Fluid Flow</Cat>
      </Categories>
      <ItemDefinitions>
        <Double Name="foo"/>
        <Block Name="B1"/>
        <String Name="bar"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Type2">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="B1"/>
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
