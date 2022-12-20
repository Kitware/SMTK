<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">
  <Includes>
    <File>ItemBlockTestGlobals.sbt</File>
    <File>ItemBlockTestGlobals2.sbt</File>
  </Includes>
  <Categories>
    <Cat>Solid Mechanics</Cat>
    <Cat>Fluid Flow</Cat>
    <Cat>Heat Transfer</Cat>
  </Categories>

<!--
Test for ItemBlocks and verifies how blocks can be overridden

The local ItemBlock B1 defined here should override any being exported by
the included files
-->
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
    <AttDef Type="Type0">
      <Categories>
        <Cat>Fluid Flow</Cat>
      </Categories>
      <ItemDefinitions>
        <Double Name="foo"/>
        <Block Name="B1"/>
        <String Name="bar"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Type1">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="B1"/>
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Type2">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="B1" Namespace="globals1"/>
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Type3">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="B1" Namespace="globals2"/>
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Type4">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="C1" Namespace="globals1"/>
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Type5">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="C1"/>
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
