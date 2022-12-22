<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">
<!--
Test file that defines 2 Item Blocks:
globals1::B1 - demonstrates that an Item Block can be overridden in another include file
              (in this case ItemBlockTestGlobals2.sbt)
globals1::C1 - demonstrates that defining a namespace at the ItemBlocks level works
-->
  <Categories>
    <Cat>Solid Mechanics</Cat>
    <Cat>Fluid Flow</Cat>
    <Cat>Heat Transfer</Cat>
  </Categories>

  <ItemBlocks Namespace="globals1">
    <Block Name="B1" Export="true">
      <ItemDefinitions>
        <String Name="s1-globals1">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i1-globals1"/>
      </ItemDefinitions>
    </Block>
    <Block Name="C1" Export="true">
      <ItemDefinitions>
        <String Name="s2-globals1">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i2-globals1"/>
      </ItemDefinitions>
    </Block>
  </ItemBlocks>

</SMTK_AttributeResource>
