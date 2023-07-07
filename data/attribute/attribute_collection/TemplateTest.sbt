<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="7">

<!--
Test for Templates showing how they can be used to set defaults
and Discrete Information
-->

  <Templates>

    <Template Name="SimpleStringDefault">
      <Parameters>
        <Param Name="a">dog</Param>
      </Parameters>
      <Contents>
        <DefaultValue>{a}</DefaultValue>
      </Contents>
    </Template>

    <Template Name = "DiscreteStringInfo">
      <Parameters>
        <Param Name="defaultIndex">0</Param>
      </Parameters>
      <Contents>
        <ChildrenDefinitions>
          <Double Name="value" Label="Value">
            <RangeInfo>
              <Min Inclusive="false">0</Min>
            </RangeInfo>
          </Double>
          <Double Name="temp" Label="Reference Temp">
          </Double>
        </ChildrenDefinitions>
        <DiscreteInfo DefaultIndex="{defaultIndex}">
          <Structure>
            <Value>Normal</Value>
            <Items>
              <Item>value</Item>
            </Items>
          </Structure>
          <Structure>
            <Value>Boussinesq</Value>
            <Items>
              <Item>value</Item>
              <Item>temp</Item>
            </Items>
          </Structure>
        </DiscreteInfo>
      </Contents>
    </Template>

    <Template Name="SimpleAttribute">
      <Parameters>
        <Param Name="type"/>
      </Parameters>
      <Contents>
        <AttDef Type="{type}">
          <ItemDefinitions>
            <String Name="s1">
              <Template Name="SimpleStringDefault">
                <Param Name="a">cat</Param>
              </Template>
            </String>
            <String Name="s2">
              <Template Name="SimpleStringDefault"/>
            </String>
            <String Name="s3">
              <Template Name="DiscreteStringInfo">
                <Param Name="defaultIndex">1</Param>
              </Template>
            </String>
          </ItemDefinitions>
        </AttDef>
      </Contents>
    </Template>
  </Templates>

  <Definitions>
    <Template Name="SimpleAttribute">
      <Param Name="type">A</Param>
    </Template>
  </Definitions>
</SMTK_AttributeResource>
