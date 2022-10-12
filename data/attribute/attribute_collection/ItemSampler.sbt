<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6" >
  <!--**********  Category and Analysis Information ***********-->
  <Categories>
    <Cat>Constituent</Cat>
    <Cat>Flow</Cat>
    <Cat>General</Cat>
    <Cat>Heat</Cat>
    <Cat>Time</Cat>
    <Cat>Veg</Cat>
  </Categories>
  <ActiveCategories Enabled="false" />
  <Analyses>
    <Analysis Type="Constituent Transport">
      <Cat>Constituent</Cat>
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Heat</Cat>
      <Cat>Time</Cat>
    </Analysis>
    <Analysis Type="CFD Flow with Heat Transfer">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Heat</Cat>
      <Cat>Time</Cat>
    </Analysis>
    <Analysis Type="CFD Flow">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Time</Cat>
    </Analysis>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="AttributeComponentDef">
      <CategoryInfo InheritanceMode="And" Combination="And" />
      <ItemDefinitions>
        <Component Name="BaseDefItem" Label="BaseDefItem" Version="0" EnforceCategories="false" Role="-2" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="And" Combination="And" />
          <Accepts>
            <Resource Name="smtk::attribute::Resource" Filter="attribute[type='BaseDef']" />
          </Accepts>
          <Rejects />
          <ComponentLabels CommonLabel="A reference to another attribute" />
        </Component>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="BaseDef" Label="BaseDef">
      <CategoryInfo InheritanceMode="And" Combination="And" />
      <ItemDefinitions>
        <Int Name="TEMPORAL" Label="Time" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="And" Combination="And">
            <Include Combination="Or">
              <Cat>Time</Cat>
            </Include>
          </CategoryInfo>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Seconds">0</Value>
            <Value Enum="Minutes">1</Value>
            <Value Enum="Hours">2</Value>
            <Value Enum="Days">3</Value>
          </DiscreteInfo>
        </Int>
        <Int Name="IntItem2" Label="IntItem2" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="And" Combination="And">
            <Include Combination="Or">
              <Cat>Heat</Cat>
            </Include>
          </CategoryInfo>
          <DefaultValue>10</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Derived1">
      <CategoryInfo InheritanceMode="And" Combination="And" />
      <AssociationsDef Name="Derived1Associations" Label="Derived1Associations" Version="0" EnforceCategories="false" Role="-1" NumberOfRequiredValues="0">
        <CategoryInfo InheritanceMode="And" Combination="And" />
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="model|nodim" />
        </Accepts>
        <Rejects />
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="DoubleItem1" Label="DoubleItem1" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="And" Combination="And">
            <Include Combination="Or">
              <Cat>Veg</Cat>
            </Include>
          </CategoryInfo>
          <ExpressionType>ExpDef</ExpressionType>
        </Double>
        <Double Name="DoubleItem2" Label="DoubleItem2" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="And" Combination="And">
            <Include Combination="Or">
              <Cat>Constituent</Cat>
            </Include>
          </CategoryInfo>
          <ChildrenDefinitions>
            <Double Name="Child1" Label="Child1" Version="0" NumberOfRequiredValues="1">
              <CategoryInfo InheritanceMode="And" Combination="And" />
            </Double>
            <Int Name="Child2" Label="Child2" Version="0" NumberOfRequiredValues="1">
              <CategoryInfo InheritanceMode="And" Combination="And" />
            </Int>
            <String Name="Child3" Label="Child3" Version="0" NumberOfRequiredValues="1">
              <CategoryInfo InheritanceMode="And" Combination="And" />
            </String>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="A">0</Value>
              <Items>
                <Item>Child1</Item>
                <Item>Child3</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="B">1</Value>
              <Items>
                <Item>Child3</Item>
                <Item>Child2</Item>
              </Items>
            </Structure>
            <Value Enum="C">2</Value>
          </DiscreteInfo>
        </Double>
        <Void Name="VoidItem" Label="Option 1" Version="0" Optional="true" IsEnabledByDefault="false">
          <CategoryInfo InheritanceMode="And" Combination="And" />
        </Void>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Derived2" Label="Derived2" BaseType="Derived1" Version="0" Unique="false">
      <CategoryInfo InheritanceMode="And" Combination="And" />
      <AssociationsDef Name="Derived2Associations" Label="Derived2Associations" Version="0" EnforceCategories="false" Role="-1" NumberOfRequiredValues="0">
        <CategoryInfo InheritanceMode="And" Combination="And" />
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="volume" />
        </Accepts>
        <Rejects />
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="StringItem1" Label="StringItem1" Version="0" NumberOfRequiredValues="1" MultipleLines="true">
          <CategoryInfo InheritanceMode="And" Combination="And">
            <Include Combination="Or">
              <Cat>Flow</Cat>
            </Include>
          </CategoryInfo>
        </String>
        <String Name="StringItem2" Label="StringItem2" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="And" Combination="And">
            <Include Combination="Or">
              <Cat>General</Cat>
            </Include>
          </CategoryInfo>
          <DefaultValue>Default</DefaultValue>
        </String>
        <Directory Name="DirectoryItem" Label="DirectoryItem" Version="0" NumberOfRequiredValues="1" ShouldExist="true" ShouldBeRelative="true">
          <CategoryInfo InheritanceMode="And" Combination="And" />
        </Directory>
        <File Name="FileItem" Label="FileItem" Version="0" NumberOfRequiredValues="1" ShouldBeRelative="true">
          <CategoryInfo InheritanceMode="And" Combination="And" />
        </File>
        <Group Name="GroupItem" Label="GroupItem" Version="0" NumberOfRequiredGroups="1">
          <CategoryInfo InheritanceMode="And" Combination="And" />
          <ItemDefinitions>
            <File Name="File1" Label="File1" Version="0" NumberOfRequiredValues="1">
              <CategoryInfo InheritanceMode="And" Combination="And" />
            </File>
            <Group Name="SubGroup" Label="SubGroup" Version="0" NumberOfRequiredGroups="1">
              <CategoryInfo InheritanceMode="And" Combination="And" />
              <ItemDefinitions>
                <String Name="GroupString" Label="GroupString" Version="0" NumberOfRequiredValues="1">
                  <CategoryInfo InheritanceMode="And" Combination="And">
                    <Include Combination="Or">
                      <Cat>Flow</Cat>
                      <Cat>General</Cat>
                    </Include>
                  </CategoryInfo>
                  <DefaultValue>Something Cool</DefaultValue>
                </String>
              </ItemDefinitions>
            </Group>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="ExpDef" Label="ExpDef" BaseType="" Version="0" Unique="false">
      <CategoryInfo InheritanceMode="And" Combination="And" />
      <BriefDescription>Sample Expression</BriefDescription>
      <DetailedDescription>Sample Expression for testing
There is not much here!</DetailedDescription>
      <ItemDefinitions>
        <String Name="Expression String" Label="Expression String" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="And" Combination="And" />
          <DefaultValue>sample</DefaultValue>
        </String>
        <String Name="Aux String" Label="Aux String" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="And" Combination="And" />
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <!--********** Workflow Views ***********-->
  <Views>
    <View Type="Group" Name="TopLevel" FilterByAdvanceLevel="true"  Style="Tiled" TopLevel="true">
      <Views>
        <View Title="Test" />
        <View Title="Exp" />
      </Views>
    </View>
    <View Type="Instanced" Name="Test">
      <InstancedAttributes>
        <Att Name="testAtt" Type="Derived2" />
      </InstancedAttributes>
    </View>
    <View Type="Instanced" Name="Exp">
      <InstancedAttributes>
        <Att Name="Exp1" Type="ExpDef" />
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
