<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">
  <Categories>
    <Cat>Fluid Flow</Cat>
    <Cat>Heat Transfer</Cat>
    <Cat>Void Material</Cat>
  </Categories>

  <Analyses>
    <Analysis Type="Heat Transfer">
      <Cat>Heat Transfer</Cat>
    </Analysis>
    <Analysis Type="Fluid Flow">
      <Cat>Fluid Flow</Cat>
    </Analysis>
    <Analysis Type="Include Void Material">
      <Cat>Void Material</Cat>
    </Analysis>
  </Analyses>

  <Definitions>
    <AttDef Type="material" BaseType="" Abstract="true" Unique="true" />

    <AttDef Type="material.real" BaseType="material" Label="Material"/>
    <AttDef Type="material.solid" BaseType="material.real" Label=" Solid Material">
      <ItemDefinitions>
        <Double Name="Density">
          <Categories>
            <Cat>Fluid Flow</Cat>
            <Cat>Heat Transfer</Cat>
          </Categories>
          <DefaultValue>0.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="material.liquid" BaseType="material.real" Label=" Liquid Material">
      <ItemDefinitions>
        <Double Name="Viscosity">
          <Categories>
            <Cat>Fluid Flow</Cat>
          </Categories>
          <DefaultValue>0.001</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="material.void" BaseType="material" Label="Void">
      <Categories>
        <Cat>Void Material</Cat>
      </Categories>
    </AttDef>

    <AttDef Type="body">
      <Categories>
        <Cat>Fluid Flow</Cat>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Component Name="material" EnforceCategories="true">
          <Accepts>
            <Resource Name="smtk::attribute::Resource" Filter="attribute[type='material']" />
          </Accepts>
          <ChildrenDefinitions>
            <Double Name="initialFlow" Label="initialFlow" Version="0" NumberOfRequiredValues="3">
              <CategoryInfo Inherit="true" Combination="All" />
            </Double>
            <Double Name="initialTemp" Label="initialTemp" Version="0" NumberOfRequiredValues="1">
              <CategoryInfo Inherit="true" Combination="All" />
            </Double>
          </ChildrenDefinitions>
          <ConditionalInfo>
            <Condition Resource="smtk::attribute::Resource" Component="attribute[type='material.solid']">
              <Items>
                <Item>initialTemp</Item>
              </Items>
            </Condition>
            <Condition Component="attribute[type='material.liquid']">
              <Items>
                <Item>initialTemp</Item>
                <Item>initialFlow</Item>
              </Items>
            </Condition>
          </ConditionalInfo>
        </Component>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Attributes>
    <Att Name="|Void" Type="material.void" />
  </Attributes>

  <Views>
    <View Type="Group" Name="Category Test" TopLevel="true" TabPosition="North"
        FilterByAdvanceLevel="false" FilterByCategory="false">
      <Views>
        <View Title="Select Modules" />
        <View Title="Materials"/>
        <View Title="Bodies"/>
      </Views>
      </View>
    <View Type="Analysis" Name="Select Modules" />

    <View Type="Attribute" Name="Materials">
      <AttributeTypes>
        <Att Type="material.real" />
      </AttributeTypes>
    </View>

    <View Type="Attribute" Name="Bodies">
      <AttributeTypes>
        <Att Type="body"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
