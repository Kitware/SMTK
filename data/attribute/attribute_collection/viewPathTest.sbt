<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Categories Default="Any">
  </Categories>
  <Definitions>
    <AttDef Type="test">
      <ItemDefinitions>
        <Double Name="a" Label="A with Spinner"/>
        <Int Name="b" Label="B - Read only"/>
        <String Name="c" Label="C Discrete">
          <ChildrenDefinitions>
            <Int Name="d" Label="D with width 20"/>
            <Double Name="e" Label="E with Spinner" />
          </ChildrenDefinitions>
          <DiscreteInfo>
            <Structure>
              <Value Enum="D Mode">d</Value>
              <Items>
                <Item>d</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="E Mode">d</Value>
              <Items>
                <Item>e</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="D and E Mode">d</Value>
              <Items>
                <Item>d</Item>
                <Item>e</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
        <Group Name="f" Label="F Group" Extensible="true" >
          <ItemDefinitions>
            <Double Name="f-a" Label="F-A with Spinner"/>
            <Int Name="f-b" Label="F-B with width 10"/>
            <String Name="f-c" Label="F-C Discrete">
              <ChildrenDefinitions>
                <Int Name="f.d" Label="F.D with width 0"/>
                <Double Name="f.e" Label="F.E with Spinner" />
                <String Name="f.f" Label="F.F Discrete">
                  <ChildrenDefinitions>
                    <Int Name="f.f.g" Label="F.F.G ReadOnly"/>
                    <Double Name="f.f.h" Label="F.F.H with width 0" />
                  </ChildrenDefinitions>
                  <DiscreteInfo>
                    <Structure>
                      <Value Enum="G Mode">d</Value>
                      <Items>
                        <Item>f.f.g</Item>
                      </Items>
                    </Structure>
                    <Structure>
                      <Value Enum="H Mode">d</Value>
                      <Items>
                        <Item>f.f.h</Item>
                      </Items>
                    </Structure>
                    <Structure>
                      <Value Enum="G and H Mode">d</Value>
                      <Items>
                        <Item>f.f.g</Item>
                        <Item>f.f.h</Item>
                      </Items>
                    </Structure>
                  </DiscreteInfo>
                </String>
              </ChildrenDefinitions>
              <DiscreteInfo>
                <Structure>
                  <Value Enum="D Mode">d</Value>
                  <Items>
                    <Item>f.d</Item>
                    <Item>f.f</Item>
                  </Items>
                </Structure>
                <Structure>
                  <Value Enum="E Mode">d</Value>
                  <Items>
                    <Item>f.e</Item>
                    <Item>f.f</Item>
                  </Items>
                </Structure>
                <Structure>
                  <Value Enum="D and E Mode">d</Value>
                  <Items>
                    <Item>f.d</Item>
                    <Item>f.e</Item>
                    <Item>f.f</Item>
                  </Items>
                </Structure>
              </DiscreteInfo>
            </String>
          </ItemDefinitions>
        </Group>
        <Int Name="g" Label="G - No Style"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Instanced" Title="Grammar Test" TopLevel="true">
      <InstancedAttributes>
        <Att Name="Test Attribute" Type="test">
          <ItemViews>
            <View Item="a" Type="Default" Option="SpinBox"/>
            <View Path="/b" ReadOnly="true"/>
            <View Path="/c">
              <ItemViews>
                <View Path="/d" FixedWidth="20"/>
                <View Item="e" Option="SpinBox"/>
              </ItemViews>
            </View>
            <View Path="/f/f-a" Option="SpinBox"/>
            <View Path="/f/f-b" FixedWidth="10"/>
            <View Path="/f/f-c/f.d" FixedWidth="0"/>
            <View Path="/f/f-c/f.e" Option="SpinBox"/>
            <View Path="/f/f-c/f.f/f.f.g" ReadOnly="true"/>
            <View Path="/f/f-c/f.f/f.f.h" FixedWidth="0"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
