<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">
<!--
Test file that defines 4 Item Blocks:
  globals1::B1 - this overrides an Item Block defined in ItemBlockTestGlobals.sbt
  ::B1 (B1 in the global namespace) - this is used to verify that ItemBlockTest.sbt can locally override it
  globals2::B1 - this demonstrate that we can define B1 in a different namespace
  ::C1 - this demonstrates that we can export something to global namespace
-->
  <Categories>
    <Cat>Solid Mechanics</Cat>
    <Cat>Fluid Flow</Cat>
    <Cat>Heat Transfer</Cat>
  </Categories>

  <ItemBlocks>
    <Block Name="B1" Export="true" Namespace="globals1">
      <ItemDefinitions>
        <String Name="s1-globals2">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i1-globals2"/>
      </ItemDefinitions>
    </Block>
    <Block Name="B1" Export="true">
      <ItemDefinitions>
        <String Name="s2-globals2">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i2-globals1"/>
      </ItemDefinitions>
    </Block>
    <Block Name="B1" Export="true" Namespace="globals2">
      <ItemDefinitions>
        <String Name="s3-globals2">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i3-globals2"/>
      </ItemDefinitions>
    </Block>
    <Block Name="C1" Export="true">
      <ItemDefinitions>
        <String Name="s4-globals2">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i4-globals2"/>
      </ItemDefinitions>
    </Block>
  </ItemBlocks>

</SMTK_AttributeResource>
