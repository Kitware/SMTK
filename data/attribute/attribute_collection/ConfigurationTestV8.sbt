<?xml version="1.0"?>
<!--Created by XmlV8StringWriter-->
<SMTK_AttributeResource Version="8" ID="8fe66bab-ac3a-4ee8-bf6c-f371946f58c0" NameSeparator="-" DisplayHint="true">
  <!--**********  Category and Analysis Information ***********-->
  <Categories>
    <Cat>Enclosure Radiation</Cat>
    <Cat>Fluid Flow</Cat>
    <Cat>Heat Transfer</Cat>
    <Cat>Induction Heating</Cat>
    <Cat>Solid Mechanics</Cat>
  </Categories>
  <ActiveCategories Enabled="false" />
  <Analyses>
    <Analysis Type="EC2">
      <Cat>ec2</Cat>
    </Analysis>
    <Analysis Type="EC1">
      <Cat>ec1</Cat>
    </Analysis>
    <Analysis Type="Solid Mechanics">
      <Cat>Solid Mechanics</Cat>
    </Analysis>
    <Analysis Type="Fluid Flow">
      <Cat>Fluid Flow</Cat>
    </Analysis>
    <Analysis Type="Heat Transfer">
      <Cat>Heat Transfer</Cat>
    </Analysis>
    <Analysis Type="Induction Heating" BaseType="Heat Transfer">
      <Cat>Induction Heating</Cat>
    </Analysis>
    <Analysis Type="Enclosure Radiation" BaseType="Heat Transfer">
      <Cat>Enclosure Radiation</Cat>
    </Analysis>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="description" Label="Description" BaseType="" Version="0" Unique="false" IgnoreCategories="false">
      <CategoryExpression InheritanceMode="Or" />
      <ItemDefinitions>
        <String Name="s1" Label="Advance Level and Enum Test String" Version="0" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="Or" />
          <DiscreteInfo>
            <Structure>
              <Value Enum="e1" AdvanceLevel="1">a</Value>
              <CategoryExpression>('ec1')</CategoryExpression>
            </Structure>
            <Structure>
              <Value Enum="e2">b</Value>
              <CategoryExpression>('ec2')</CategoryExpression>
            </Structure>
            <Structure>
              <Value Enum="e3" AdvanceLevel="1">c</Value>
              <CategoryExpression PassMode="All" />
            </Structure>
          </DiscreteInfo>
        </String>
        <String Name="description" Label="text:" Version="0" NumberOfRequiredValues="1" MultipleLines="true">
          <CategoryExpression InheritanceMode="Or">
            ('Enclosure Radiation' + 'Induction Heating')
          </CategoryExpression>
          <BriefDescription>Text added to top of input file</BriefDescription>
          <DefaultValue># Truchas simulation</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="numerics" Label="Numerics" BaseType="" Version="0" Unique="true" IgnoreCategories="false">
      <CategoryExpression InheritanceMode="Or" />
      <ItemDefinitions>
        <Group Name="velocity-group" Label="Velocity" Version="0" NumberOfRequiredGroups="1" Extensible="true">
          <CategoryExpression InheritanceMode="Or" />
          <ComponentLabels CommonLabel="" />
          <ItemDefinitions>
            <Component Name="uniqueTest" Label="Unique Test" Version="0" EnforceCategories="false" Role="0" NumberOfRequiredValues="1">
              <CategoryExpression InheritanceMode="Or">('Heat Transfer')</CategoryExpression>
              <Accepts>
                <Resource Name="smtk::model::Resource" Filter="face" />
              </Accepts>
              <Rejects />
            </Component>
            <Double Name="velocity-value" Label="Velocity" Version="0" NumberOfRequiredValues="4">
              <CategoryExpression InheritanceMode="Or">('Fluid Flow')</CategoryExpression>
              <ComponentLabels>
                <Label>t</Label>
                <Label>u</Label>
                <Label>v</Label>
                <Label>w</Label>
              </ComponentLabels>
              <DefaultValue>0.0</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
        <Double Name="dt_init" Label="Dt_Init" Version="0" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="Or">('Fluid Flow')</CategoryExpression>
          <BriefDescription>Integration time step value used for
the first computation cycle</BriefDescription>
          <DefaultValue>1.0e-6</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_grow" Label="Dt_Grow" Version="0" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="Or">('Fluid Flow')</CategoryExpression>
          <BriefDescription>A factor to multiply the current integration time step
when estimating the next cycle time step.</BriefDescription>
          <DefaultValue>1.05</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_max" Label="Dt_Max" Version="0" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="Or">('Solid Mechanics')</CategoryExpression>
          <BriefDescription>Maximum allowable value for the time step.</BriefDescription>
          <DefaultValue>10.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_min" Label="Dt_Min" Version="0" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="Or">('Solid Mechanics')</CategoryExpression>
          <BriefDescription>Minimum allowable value for the time step.</BriefDescription>
          <DefaultValue>1.0e-6</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="outputs" Label="Outputs" BaseType="" Version="0" Unique="true" IgnoreCategories="false">
      <CategoryExpression InheritanceMode="Or" />
      <ItemDefinitions>
        <Double Name="start-time" Label="Start Time" Version="0" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="Or">
            ('Induction Heating' ∨ 'Solid Mechanics')
          </CategoryExpression>
          <DefaultValue>0.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="end-time" Label="End Time" Version="0" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="Or">
            ('Induction Heating' | 'Solid Mechanics')
          </CategoryExpression>
          <DefaultValue>1.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="output-dt" Label="Initial Output Delta-Time" Version="0" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="Or">('Heat Transfer')</CategoryExpression>
          <DefaultValue>1.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Group Name="output-times" Label="Additional Output Control" Version="0" NumberOfRequiredGroups="0" Extensible="true">
          <CategoryExpression InheritanceMode="Or" />
          <ComponentLabels CommonLabel="" />
          <ItemDefinitions>
            <Double Name="time" Label="Output Times" Version="0" NumberOfRequiredValues="2">
              <CategoryExpression InheritanceMode="Or">('Fluid Flow')</CategoryExpression>
              <ComponentLabels>
                <Label>After time:</Label>
                <Label>Use delta time:</Label>
              </ComponentLabels>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="simulation-control" Label="Simulation Control" BaseType="" Version="0" Unique="true" IgnoreCategories="false">
      <CategoryExpression InheritanceMode="Or" />
      <ItemDefinitions>
        <Group Name="simulation-control" Label="Simulation Control" Version="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredGroups="1">
          <CategoryExpression InheritanceMode="Or" />
          <BriefDescription>This item is superseded by the displacement sequence when
moving-enclosure radiation is enabled</BriefDescription>
          <ItemDefinitions>
            <Double Name="phase-start-times" Label="Phase Start Times" Version="0" NumberOfRequiredValues="1" Extensible="true">
              <CategoryExpression InheritanceMode="Or">('Heat Transfer')</CategoryExpression>
              <BriefDescription>The list of starting times of each of the phases</BriefDescription>
            </Double>
            <Double Name="phase-init-dt-factor" Label="Phase Init Dt Factor" Version="0" NumberOfRequiredValues="1">
              <CategoryExpression InheritanceMode="Or">('Fluid Flow' | 'Heat Transfer' | 'Solid Mechanics')
              </CategoryExpression>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Exclusions />
  <!--********** Workflow Views ***********-->
  <Views>
    <View Type="Group" Name="TopLevel" ConfigurationLabel="My Configurations:" ConfigurationType="Analysis" FilterByAdvanceLevel="true" TabPosition="North" TopLevel="true" UseConfigurations="true">
      <Views>
        <View Title="Test" />
        <View Title="Configurations" />
      </Views>
    </View>
    <View Type="Attribute" Name="Configurations" IgnoreCategories="true">
      <AttributeTypes>
        <Att Type="Analysis" />
      </AttributeTypes>
    </View>
    <View Type="Instanced" Name="Description">
      <InstancedAttributes>
        <Att Name="description" Type="description" />
      </InstancedAttributes>
    </View>
    <View Type="Instanced" Name="General">
      <InstancedAttributes>
        <Att Name="numerics-att" Type="numerics">
          <ItemViews>
            <View InsertMode="Prepend" Item="velocity-group" Type="Default" />
            <View EditPrecision="6" Item="dt_init" Precision="2" Type="Default" />
            <View EditPrecision="6" Item="dt_max" Notation="Fixed" Precision="6" Type="Default" />
            <View EditPrecision="6" Item="dt_min" Notation="Scientific" Precision="6" Type="Default" />
          </ItemViews>
        </Att>
        <Att Name="outputs-att" Type="outputs" />
        <Att Name="simulation-control-att" Type="simulation-control" />
      </InstancedAttributes>
    </View>
    <View Type="Group" Name="Test" Style="Tiled">
      <Views>
        <View Title="Description" />
        <View Title="General" />
      </Views>
    </View>
  </Views>
  <UniqueRoles>
    <Role ID="0" />
    <Role ID="10" />
  </UniqueRoles>
</SMTK_AttributeResource>
