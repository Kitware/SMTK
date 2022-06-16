<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">

  <ItemBlocks>
    <Block Name="Labeled">
      <ItemDefinitions>
        <Int Name="Int1">
          <DefaultValue>1</DefaultValue>
        </Int>
        <Int Name="Int2" NumberOfRequiredValues="2">
          <DefaultValue>1</DefaultValue>
        </Int>
        <Component Name="ReferenceItem">
          <Accepts>
            <Resource Name="smtk::attribute::Resource" Filter="attribute[type='Example']" />
          </Accepts>
        </Component>
        <DateTime Name="DateTime"></DateTime>
        <File Name="FileItem" ShouldExist="true" FileFilters="(*)"></File>
        <Group Name="GroupItem" Extensible="false">
          <ItemDefinitions>
            <Double Name="e"><DefaultValue>2.71828</DefaultValue></Double>
            <Double Name="pi"><DefaultValue>3.14159</DefaultValue></Double>
          </ItemDefinitions>
        </Group>
        <Void Name="OptionalItem" Optional="true" IsEnabledByDefault="false" />
        <Void Name="VoidItem" Optional="false" />
      </ItemDefinitions>
    </Block>
  </ItemBlocks>

  <Definitions>
    <AttDef Type="Case1">
      <ItemDefinitions>
        <Void Name="1. BY DEFAULT, VALUE ITEMS USE HORIZONAL LAYOUT:" />
        <Double Name="OneValue"><DefaultValue>1.1</DefaultValue></Double>
        <Double Name="ThreeValue" NumberOfRequiredValues="3">
          <DefaultValue>1.1,2.2,3.3</DefaultValue>
        </Double>
        <String Name="Discrete" NumberOfRequiredValues="2">
          <DiscreteInfo DefaultIndex="0">
            <Value>alpha</Value>
            <Value>omega</Value>
          </DiscreteInfo>
        </String>
        <Void Name="2. BUT YOU CAN USE &lt;ItemView Layout=&quot;Vertical&quot;&gt; TO CHANGE THIS:" />
        <Double Name="ThreeVertical" NumberOfRequiredValues="3">
          <DefaultValue>4.4,5.5,6.6</DefaultValue>
        </Double>
        <String Name="DiscreteVertical" NumberOfRequiredValues="2">
          <DiscreteInfo DefaultIndex="0">
            <Value>beta</Value>
            <Value>gamma</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="Case2">
      <ItemDefinitions>
        <Void Name="1. EXTENSIBLE ITEMS *ALWAYS* USE VERTICAL LAYOUT:" />
        <Void Name="(ItemView &quot;Layout&quot; options are ignored)" />
        <Double Name="ThreeExt" Extensible="true" NumberOfRequiredValues="3">
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <String Name="DiscreteExt" Extensible="true" NumberOfRequiredValues="2">
          <DiscreteInfo DefaultIndex="0">
            <Value>beta</Value>
            <Value>gamma</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="Case3">
      <ItemDefinitions>
        <Void Name="1. BY DEFAULT, DISCRETE ITEMS WITH ACTIVE CHILDREN USE VERTICAL LAYOUT:" />
        <String Name="Select1" Label="Select">
          <ChildrenDefinitions>
            <Block Name="Labeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>All</Value>
              <Items>
                <Item>Int1</Item>
                <Item>Int2</Item>
                <Item>ReferenceItem</Item>
                <Item>DateTime</Item>
                <Item>FileItem</Item>
                <Item>GroupItem</Item>
                <Item>OptionalItem</Item>
                <Item>VoidItem</Item>
              </Items>
            </Structure>
            <Value>No Children</Value>
            <Structure>
              <Value>One Child</Value>
              <Items>
                <Item>Int1</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>Two Children</Value>
              <Items>
                <Item>Int1</Item>
                <Item>Int2</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>Reference Item</Value>
              <Items>
                <Item>ReferenceItem</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>Date Time</Value>
              <Items>
                <Item>DateTime</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>File Item</Value>
              <Items>
                <Item>FileItem</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>Group Item</Value>
              <Items>
                <Item>GroupItem</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>Optional Item</Value>
              <Items>
                <Item>OptionalItem</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>Void Item (not optional)</Value>
              <Items>
                <Item>VoidItem</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <Void Name="2. BUT IF EACH DISCRETE VALUE HAS SINGLE ACTIVE CHILD WITH BLANK LABEL =&gt; HORIZONTAL LAYOUT:" />
        <String Name="Select2" Label="Select">
          <ChildrenDefinitions>
            <Int Name="Int1" Label=" ">
              <DefaultValue>1</DefaultValue>
            </Int>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="1">
            <Value>No Children</Value>
            <Structure>
              <Value>One Child</Value>
              <Items>
                <Item>Int1</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <Void Name="3. YOU CAN ALSO USE &lt;ItemView Layout=&quot;Horizontal&quot;&gt; TO CHANGE THIS:" />
        <String Name="Select3" Label="Select">
          <ChildrenDefinitions>
            <Int Name="Int3"><DefaultValue>1</DefaultValue></Int>
            <Int Name="Int4"><DefaultValue>2</DefaultValue></Int>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="2">
            <Value>No Children</Value>
            <Structure>
              <Value>One Child</Value>
              <Items>
                <Item>Int3</Item>
              </Items>
            </Structure>
            <Structure>
              <Value>Two Children</Value>
              <Items>
                <Item>Int3</Item>
                <Item>Int4</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="Example" />
   </Definitions>

  <Attributes>
    <Att Type="Example" Name="Example" />
  </Attributes>

  <Views>
    <View Type="Group" Title="Label Examples" TabPosition="North" TopLevel="true"
          FilterByAdvanceLevel="false" FilterByCategory="false"
          UseScrollingContainer="false">
      <Views>
        <View Title="Case1" />
        <View Title="Case2" />
        <View Title="Case3" />
      </Views>
    </View>

    <View Type="Instanced" Title="Case1" Label="Value Items">
      <InstancedAttributes>
        <Att Type="Case1" Name="Case1">
          <ItemViews>
            <View Path="/ThreeVertical" Layout="Vertical" />
            <View Path="/DiscreteVertical" Layout="Vertical" />
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>

    <View Type="Instanced" Title="Case2" Label="Extensible">
      <InstancedAttributes>
        <Att Type="Case2" Name="Case2" />
      </InstancedAttributes>
    </View>

    <View Type="Instanced" Title="Case3" Label="Children">
      <InstancedAttributes>
        <Att Type="Case3" Name="Case3">
          <ItemViews>
            <View Path="/Select3" Layout="Horizontal" />
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
