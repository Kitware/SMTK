
<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">

  <!--**********  Category and Analysis Information ***********-->

  <Categories>
    <Cat>a</Cat>
    <Cat>b</Cat>
    <Cat>c</Cat>
    <Cat>d</Cat>
    <Cat>e</Cat>
    <Cat>f</Cat>
  </Categories>
  <Analyses>
    <Analysis Type="Required Analysis" Required="true">
      <Cat>alpha</Cat>
    </Analysis>
    <Analysis Type="Require Exclusive" BaseType="Required Analysis" Exclusive="True"  Required="true">
      <Cat>a</Cat>
    </Analysis>
    <Analysis Type="Simple" Label="Simple Analysis Label" BaseType="Require Exclusive" Exclusive="True">
    </Analysis>
    <Analysis Type="Simple Option 1" Label="Option 1" BaseType="Simple">
      <Cat>b</Cat>
    </Analysis>
    <Analysis Type="Simple Option 2" Label="Option 2" BaseType="Simple">
      <Cat>c</Cat>
    </Analysis>
    <Analysis Type="Simple Option 3" Label="Option 3" BaseType="Simple">
      <Cat>d</Cat>
    </Analysis>
    <Analysis Type="Complex" Label="Complex Analysis Label" BaseType="Require Exclusive" Exclusive="True">
    </Analysis>
    <Analysis Type="Complex Option 1" Label="Option 1" BaseType="Complex">
      <Cat>e</Cat>
    </Analysis>
    <Analysis Type="Complex Option 2" Label="Option 2" BaseType="Complex">
      <Cat>f</Cat>
    </Analysis>
  </Analyses>

  <Definitions>
    <AttDef Type="Example" Label="Example Workflow">
      <ItemDefinitions>
        <String Name="a-info" Label="A Info">
          <Categories>
            <Cat>a</Cat>
          </Categories>
        </String>
        <String Name="b-info" Label="B Info">
          <Categories>
            <Cat>b</Cat>
          </Categories>
        </String>
        <String Name="c-info" Label="C Info">
          <Categories>
            <Cat>c</Cat>
          </Categories>
        </String>
        <String Name="d-info" Label="D Info">
          <Categories>
            <Cat>d</Cat>
          </Categories>
        </String>
        <String Name="e-info" Label="E Info">
          <Categories>
            <Cat>e</Cat>
          </Categories>
        </String>
        <String Name="f-info" Label="F Info">
          <Categories>
            <Cat>f</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Group" Name="Main"  TopLevel="true" Style="groupbox" FilterByCategory="false" >
      <Views>
        <View Title="Analysis" />
        <View Title="Example" />
      </Views>
    </View>
    <View Type="Analysis" Title="Analysis" AnalysisAttributeName="simpleAnalysis" AnalysisAttributeType="simpleAnalysisDefinition"/>
    <View Type="Instanced" Name="Example">
      <InstancedAttributes>
        <Att Name="exampleAtt" Type="Example" />
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
