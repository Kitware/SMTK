<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">

  <!-- Category & Analysis specifications -->
  <Categories>
    <Cat>A</Cat>
    <Cat>B</Cat>
    <Cat>C</Cat>
    <Cat>D</Cat>
    <Cat>E</Cat>
    <Cat>F</Cat>
  </Categories>
<!-- Lets define some analyses with the top level exclusive
  A - non-exclusive with no derived children
  B - non-exclusive with derived children
  C - exclusive with derived children
  B-D - derived from B with no children
  B-E - derived from B with no children
  C-D - derived from C with no children
  C-E - exclusive derived from C with children
  C-E-D - derived from C-E with no children
  C-E-F - derived from C-E with no children
-->
  <Analyses Exclusive="true">
    <Analysis Type="A">
      <Cat>A</Cat>
    </Analysis>
    <Analysis Type="B">
      <Cat>B</Cat>
    </Analysis>
    <Analysis Type="C" Exclusive="true">
      <Cat>C</Cat>
    </Analysis>
    <Analysis Type="B-D" BaseType="B">
      <Cat>D</Cat>
    </Analysis>
    <Analysis Type="B-E" BaseType="B">
      <Cat>E</Cat>
    </Analysis>
    <Analysis Type="C-D" BaseType="C">
      <Cat>D</Cat>
    </Analysis>
    <Analysis Type="C-E"  Exclusive="true" BaseType="C">
      <Cat>E</Cat>
    </Analysis>
    <Analysis Type="C-E-D" BaseType="C-E">
      <Cat>D</Cat>
    </Analysis>
    <Analysis Type="C-E-F" BaseType="C-E">
      <Cat>F</Cat>
    </Analysis>
  </Analyses>
<!-- Lets create some analysis configurations
  Test A
    - Set top level to A
  Test B
    - Set top level to B
  Test B-D
    - Set top level to B and turn on D
  Test C
    - Set top level to C - should not create a configuration
  Test C-D
    - Set top level to C and select D
  Test C-E
    - Set top level to C and select E  - should not create a configuration
  Test C-E-F
    - Set top level to C and select E and then select F
-->
  <Configurations AnalysisAttributeType="Analysis">
    <Config Name="Test A" AdvanceReadLevel="5">
      <Analysis Type="A"/>
    </Config>
    <Config Name="Test B" AdvanceWriteLevel="10">
      <Analysis Type="B"/>
    </Config>
    <Config Name="Test B-D">
      <Analysis Type="B">
        <Analysis Type="B-D"/>
      </Analysis>
    </Config>
    <Config Name="Test C-D">
      <Analysis Type="C">
        <Analysis Type="C-D"/>
      </Analysis>
    </Config>
    <Config Name="Test C-E-F">
      <Analysis Type="C">
        <Analysis Type="C-E">
          <Analysis Type="C-E-F"/>
        </Analysis>
      </Analysis>
    </Config>
  </Configurations>

  <Attributes>
    <Att Name="IntiallyEmptyConfig" Type="Analysis"/>
  </Attributes>

    <Views>
    <View Type="Attribute" Title="Analysis" AnalysisAttributeName="simpleAnalysis" AnalysisAttributeType="Analysis"  TopLevel="true">
      <AttributeTypes>
        <Att Type="Analysis"/>
      </AttributeTypes>
    </View>
  </Views>

</SMTK_AttributeResource>
