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
        <Group Name="GroupItem">
          <ItemDefinitions>
            <Double Name="e"><DefaultValue>2.71828</DefaultValue></Double>
            <Double Name="pi"><DefaultValue>3.14159</DefaultValue></Double>
          </ItemDefinitions>
        </Group>
        <Void Name="OptionalItem" Optional="true" IsEnabledByDefault="false" />
        <Void Name="VoidItem" Optional="false" />
      </ItemDefinitions>
    </Block>

    <Block Name="Unlabeled">
      <ItemDefinitions>
        <Int Name="Int1" Label=" ">
          <DefaultValue>1</DefaultValue>
        </Int>
        <Int Name="Int2" Label=" " NumberOfRequiredValues="2">
          <DefaultValue>1</DefaultValue>
        </Int>
        <Component Name="ReferenceItem" Label=" ">
          <Accepts>
            <Resource Name="smtk::attribute::Resource" Filter="attribute[type='Example']" />
          </Accepts>
        </Component>
        <DateTime Name="DateTime" Label=" "></DateTime>
        <File Name="FileItem" Label=" " ShouldExist="true" FileFilters="(*)"></File>
        <Group Name="GroupItem" Label=" ">
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
    <AttDef Type="Example">
      <ItemDefinitions>
        <String Name="No Children">
          <DiscreteInfo DefaultIndex="0">
            <Value>Nada</Value>
          </DiscreteInfo>
        </String>

        <String Name="One Child">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>One Child</Value>
              <Items>
                <Item>Int1</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="Two Children">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>Two Children</Value>
              <Items>
                <Item>Int1</Item>
                <Item>Int2</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="Reference Item">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>Reference Item</Value>
              <Items>
                <Item>ReferenceItem</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="Date Time">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>Date Time</Value>
              <Items>
                <Item>DateTime</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="File Item">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>File Item</Value>
              <Items>
                <Item>FileItem</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="Group Item">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>Group Item (one subgroup)</Value>
              <Items>
                <Item>GroupItem</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="Optional Item">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>Optional Item</Value>
              <Items>
                <Item>OptionalItem</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="Void Item">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value>Void Item (not optional)</Value>
              <Items>
                <Item>VoidItem</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="Select" Label="Select (default layout)">
          <ChildrenDefinitions>
            <Block Name="Unlabeled" />
          </ChildrenDefinitions>
          <DiscreteInfo>
            <Structure>
              <Value>All</Value>
              <Items>
                <Item>Int1</Item>
                <Item>Int2</Item>
                <Item>ReferenceItem</Item>
                <Item>DateTime</Item>
                <Item>FileItem</Item>
                <Item>GroupItem</Item>
<!--                 <Item>OptionalItem</Item>
                <Item>VoidItem</Item> -->
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

        <String Name="SelectVertical" Label="Select (Vertical)">
          <ChildrenDefinitions>
            <Block Name="Labeled" />
          </ChildrenDefinitions>
          <DiscreteInfo>
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
          </DiscreteInfo>
        </String>

      </ItemDefinitions>
    </AttDef>
   </Definitions>
  <!--**********  Attribute Instances ***********-->
  <Views>
    <View Type="Instanced" Title="Label Examples" TopLevel="true">
      <InstancedAttributes>
        <Att Type="Example" Name="Example">
          <ItemViews>
            <View Path="/SelectVertical" Layout="Vertical" />
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
