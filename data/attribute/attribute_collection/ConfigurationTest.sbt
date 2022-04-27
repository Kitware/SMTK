<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">

  <!-- Category & Analysis specifications -->
  <Categories>
    <Cat>Enclosure Radiation</Cat>
    <Cat>Fluid Flow</Cat>
    <Cat>Heat Transfer</Cat>
    <Cat>Induction Heating</Cat>
    <Cat>Solid Mechanics</Cat>
    <Cat>ec1</Cat>
    <Cat>ec2</Cat>
  </Categories>

  <Analyses>
    <Analysis Type="Heat Transfer">
      <Cat>Heat Transfer</Cat>
    </Analysis>
    <Analysis Type="Enclosure Radiation" BaseType="Heat Transfer">
      <Cat>Enclosure Radiation</Cat>
    </Analysis>
    <Analysis Type="Induction Heating" BaseType="Heat Transfer">
      <Cat>Induction Heating</Cat>
    </Analysis>
    <Analysis Type="Fluid Flow">
      <Cat>Fluid Flow</Cat>
    </Analysis>
    <Analysis Type="Solid Mechanics">
      <Cat>Solid Mechanics</Cat>
    </Analysis>
    <Analysis Type="EC1">
      <Cat>ec1</Cat>
    </Analysis>
    <Analysis Type="EC2">
      <Cat>ec2</Cat>
    </Analysis>
  </Analyses>

  <UniqueRoles>
    <Role ID="10"/>
    <Role ID="0"/>
  </UniqueRoles>

  <Definitions>
    <!-- Numerics-->
    <AttDef Type="numerics" Label="Numerics" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Group Name="velocity-group" Label="Velocity" Extensible="true" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Component Name="uniqueTest" Label="Unique Test" NumberOfRequiredValues="1" Role="0">
              <Categories>
                <Cat>Heat Transfer</Cat>
              </Categories>
              <Accepts>
                <Resource Name="smtk::model::Resource" Filter="face"/>
              </Accepts>
            </Component>
            <Double Name="velocity-value" Label="Velocity" NumberOfRequiredValues="4">
              <Categories>
                <Cat>Fluid Flow</Cat>
              </Categories>
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
        <Double Name="dt_init" Label="Dt_Init">
          <Categories>
            <Cat>Fluid Flow</Cat>
          </Categories>
          <BriefDescription>Integration time step value used for
the first computation cycle</BriefDescription>
          <DefaultValue>1.0e-6</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_grow" Label="Dt_Grow">
          <BriefDescription>A factor to multiply the current integration time step
when estimating the next cycle time step.</BriefDescription>
          <Categories>
            <Cat>Fluid Flow</Cat>
          </Categories>
          <DefaultValue>1.05</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1.0</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_max" Label="Dt_Max">
          <BriefDescription>Maximum allowable value for the time step.</BriefDescription>
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
          <DefaultValue>10.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_min" Label="Dt_Min">
          <BriefDescription>Minimum allowable value for the time step.</BriefDescription>
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
          <DefaultValue>1.0e-6</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="outputs" Label="Outputs" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Double Name="start-time" Label="Start Time" Version="0">
          <Categories>
            <Cat>Induction Heating</Cat>
            <Cat>Solid Mechanics</Cat>
          </Categories>
          <DefaultValue>0.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.0</Min>
          </RangeInfo>
        </Double>
        <Double Name="end-time" Label="End Time" Version="0">
          <Categories>
            <Cat>Induction Heating</Cat>
            <Cat>Solid Mechanics</Cat>
          </Categories>
          <DefaultValue>1.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.0</Min>
          </RangeInfo>
        </Double>
        <Double Name="output-dt" Label="Initial Output Delta-Time" Version="0">
          <Categories>
            <Cat>Heat Transfer</Cat>
          </Categories>
          <DefaultValue>1.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.0</Min>
          </RangeInfo>
        </Double>
        <Group Name="output-times" Label="Additional Output Control" Extensible="true" NumberOfRequiredGroups="0">
          <ItemDefinitions>
            <Double Name="time" Label="Output Times" NumberOfRequiredValues="2">
              <Categories>
                <Cat>Fluid Flow</Cat>
              </Categories>
              <ComponentLabels>
                <Label>After time:</Label>
                <Label>Use delta time:</Label>
              </ComponentLabels>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="simulation-control" Label="Simulation Control" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Group Name="simulation-control" Label="Simulation Control" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>This item is superseded by the displacement sequence when
moving-enclosure radiation is enabled</BriefDescription>
          <ItemDefinitions>
            <Double Name="phase-start-times" Label="Phase Start Times" Extensible="true">
              <BriefDescription>The list of starting times of each of the phases</BriefDescription>
              <Categories>
                <Cat>Heat Transfer</Cat>
              </Categories>
            </Double>
            <Double Name="phase-init-dt-factor" Label="Phase Init Dt Factor">
              <Categories>
                <Cat>Fluid Flow</Cat>
                <Cat>Heat Transfer</Cat>
                <Cat>Solid Mechanics</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="description" Label="Description" BaseType="" Version="0">
      <ItemDefinitions>
       <String Name="s1" Label="Advance Level and Enum Test String" Version="0" OkToInheritCategories="true" CategoryCheckMode="Any" NumberOfRequiredValues="1">
        <DiscreteInfo>
          <Structure>
            <Value Enum="e1" AdvanceLevel="1">a</Value>
            <Categories>
              <Cat>ec1</Cat>
            </Categories>
          </Structure>
          <Structure>
            <Value Enum="e2">b</Value>
            <Categories>
              <Cat>ec2</Cat>
            </Categories>
          </Structure>
          <Value Enum="e3" AdvanceLevel="1">c</Value>
        </DiscreteInfo>
      </String>
        <String Name="description" Label="text:" MultipleLines="true">
          <BriefDescription>Text added to top of input file</BriefDescription>
          <Categories>
            <Cat>Enclosure Radiation</Cat>
            <Cat>Induction Heating</Cat>
          </Categories>
          <DefaultValue># Truchas simulation</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

 <!-- View specifications -->
  <Views>
    <View Type="Group" Title="TopLevel" TopLevel="true" TabPosition="North"
      FilterByAdvanceLevel="true" UseConfigurations="true" ConfigurationType="Analysis"
      ConfigurationLabel="My Configurations:">
      <Views>
        <View Title="Test" />
        <View Title="Configurations" />
      </Views>
    </View>

   <View Type="Attribute" Title="Configurations" IgnoreCategories="true">
      <AttributeTypes>
        <Att Type="Analysis" />
      </AttributeTypes>
    </View>

    <View Type="Group" Title="Test" Style="Tiled">
      <Views>
        <View Title="Description" />
        <View Title="General" />
      </Views>
    </View>

    <View Type="Instanced" Title="General">
      <InstancedAttributes>
        <Att Name="numerics-att" Type="numerics">
          <ItemViews>
            <View Item="velocity-group" Type="Default" InsertMode="Prepend"/>
            <View Item="dt_init" Type="Default" Precision="2" EditPrecision="6"/>
            <View Item="dt_max" Type="Default" Precision="6" EditPrecision="6" Notation="Fixed"/>
            <View Item="dt_min" Type="Default" Precision="6" EditPrecision="6" Notation="Scientific"/>
          </ItemViews>
        </Att>
        <Att Name="outputs-att" Type="outputs" />
        <Att Name="simulation-control-att" Type="simulation-control" />
      </InstancedAttributes>
    </View>

    <View Type="Instanced" Name="Description">
      <InstancedAttributes>
        <Att Name="description" Type="description" />
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
