<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6"  DisplayHint="true">
  <!--**********  Attribute Definitions ***********-->
  <Categories>
    <Cat>CFD</Cat>
    <Cat>Heat Transfer</Cat>
  </Categories>

  <Analyses>
    <Analysis Type="CFD" Required="true">
      <Cat>CFD</Cat>
    </Analysis>
    <Analysis Type="Heat Transfer" BaseType="CFD">
      <Cat>Heat Transfer</Cat>
    </Analysis>
  </Analyses>
  <Definitions>
    <AttDef Type="BoundaryCondition" Label="BoundaryCondition" Abstract="true">
      <AssociationsDef  Extensible="true">
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="face" />
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="Note">
          <CategoryInfo Combination="Or"/>
      </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Pressure" Label="Pressure" BaseType="BoundaryCondition" Unique="true">
      <CategoryInfo InheritanceMode="LocalOnly" Combination="And">
        <Include Combination="Or">
          <Cat>CFD</Cat>
        </Include>
      </CategoryInfo>
      <ItemDefinitions>
        <Double Name="Pressure" Label="Pressure" >
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="thermal" Label="Thermal" BaseType="BoundaryCondition" Unique="true">
      <CategoryInfo InheritanceMode="LocalOnly" Combination="And">
        <Include Combination="Or">
          <Cat>Heat Transfer</Cat>
        </Include>
      </CategoryInfo>
      <ItemDefinitions>
        <Double Name="temp" Label="Temperature" >
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Velocity" Label="Velocity" BaseType="BoundaryCondition" Unique="true">
      <CategoryInfo InheritanceMode="LocalOnly" Combination="And">
        <Include Combination="Or">
          <Cat>CFD</Cat>
        </Include>
      </CategoryInfo>
      <ItemDefinitions>
        <Double Name="Velocity" Label="Velocity" NumberOfRequiredValues="3">
          <ComponentLabels>
            <Label>x=</Label>
            <Label>y=</Label>
            <Label>z=</Label>
          </ComponentLabels>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Material" Label="Material"  Unique="true">
      <AssociationsDef  Extensible="true">
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="volume" />
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="Density">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>CFD</Cat>
            </Include>
          </CategoryInfo>
          <ChildrenDefinitions>
            <Double Name="value" Label="Value">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="temp" Label="Reference Temp">
            </Double>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
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
        </String>

        <Double Name="Viscosity" Label="Viscosity">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>CFD</Cat>
            </Include>
          </CategoryInfo>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>

        <Double Name="fluidOnly" Label="Fluid Only Prop">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>CFD</Cat>
            </Include>
            <Exclude Combination="Or">
              <Cat>Heat Transfer</Cat>
            </Exclude>
          </CategoryInfo>
        </Double>
        <Double Name="both" Label="Coupled Prop">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="And">
              <Cat>CFD</Cat>
               <Cat>Heat Transfer</Cat>
           </Include>
          </CategoryInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

 <Views>
    <View Type="Group" Name="TopLevel" FilterByAdvanceLevel="true" TabPosition="North" TopLevel="true">
      <Views>
        <View Title="Set Analysis" />
        <View Title="Materials" />
        <View Title="Boundary Conditions" />
      </Views>
    </View>
    <View Type="Attribute" Title="Materials">
      <AttributeTypes>
        <Att Type="Material"/>
      </AttributeTypes>
    </View>
    <View Type="Attribute" Title="Boundary Conditions">
      <AttributeTypes>
        <Att Type="BoundaryCondition"/>
      </AttributeTypes>
    </View>
     <View Type="Analysis" Name="Set Analysis" AnalysisAttributeName="analysis" AnalysisAttributeType="analysis" />
 </Views>


</SMTK_AttributeResource>
