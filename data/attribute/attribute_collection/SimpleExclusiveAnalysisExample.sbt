<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6"   DisplayHint="true">
  <!--**********  Category and Analysis Information ***********-->
  <Categories>
    <Cat>a</Cat>
    <Cat>b</Cat>
    <Cat>c</Cat>
    <Cat>d</Cat>
    <Cat>e</Cat>
    <Cat>f</Cat>
  </Categories>
  <ActiveCategories Enabled="false" />
  <Analyses>
    <Analysis Type="Required Analysis" Required="true">
      <Cat>alpha</Cat>
    </Analysis>
    <Analysis Type="Require Exclusive" Exclusive="true" Required="true" BaseType="Required Analysis">
      <Cat>a</Cat>
    </Analysis>
    <Analysis Type="Complex" Exclusive="true" BaseType="Require Exclusive" Label="Complex Analysis Label" />
    <Analysis Type="Complex Option 2" BaseType="Complex" Label="Option 2">
      <Cat>f</Cat>
    </Analysis>
    <Analysis Type="Complex Option 1" BaseType="Complex" Label="Option 1">
      <Cat>e</Cat>
    </Analysis>
    <Analysis Type="Simple" Exclusive="true" BaseType="Require Exclusive" Label="Simple Analysis Label" />
    <Analysis Type="Simple Option 3" BaseType="Simple" Label="Option 3">
      <Cat>d</Cat>
    </Analysis>
    <Analysis Type="Simple Option 2" BaseType="Simple" Label="Option 2">
      <Cat>c</Cat>
    </Analysis>
    <Analysis Type="Simple Option 1" BaseType="Simple" Label="Option 1">
      <Cat>b</Cat>
    </Analysis>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="Example" Label="Example Workflow" BaseType="" Version="0" Unique="false">
      <ItemDefinitions>
        <String Name="a-info" Label="A Info" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>a</Cat>
            </Include>
          </CategoryInfo>
        </String>
        <String Name="b-info" Label="B Info" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>b</Cat>
            </Include>
          </CategoryInfo>
        </String>
        <String Name="c-info" Label="C Info" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>c</Cat>
            </Include>
          </CategoryInfo>
        </String>
        <String Name="d-info" Label="D Info" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>d</Cat>
            </Include>
          </CategoryInfo>
        </String>
        <String Name="e-info" Label="E Info" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>e</Cat>
            </Include>
          </CategoryInfo>
        </String>
        <String Name="f-info" Label="F Info" Version="0" NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>f</Cat>
            </Include>
          </CategoryInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <!--********** Workflow Views ***********-->
  <Views>
    <View Type="Group" Name="Main" FilterByCategory="false" Style="groupbox" TopLevel="true">
      <Views>
        <View Title="Analysis" />
        <View Title="Example" />
      </Views>
    </View>
    <View Type="Analysis" Name="Analysis" AnalysisAttributeName="simpleAnalysis" AnalysisAttributeType="simpleAnalysisDefinition" />
    <View Type="Instanced" Name="Example">
      <InstancedAttributes>
        <Att Name="exampleAtt" Type="Example" />
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
