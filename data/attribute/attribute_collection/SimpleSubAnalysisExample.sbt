<?xml version="1.0"?>
<!--Created by XmlV6StringWriter-->
<SMTK_AttributeResource Version="6" DisplayHint="true">
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
    <AttDef Type="description" Label="Description" BaseType=""  Unique="false">
      <CategoryInfo InheritanceMode="Or" Combination="Or" />
      <ItemDefinitions>
        <String Name="s1" Label="Advance Level and Enum Test String"  NumberOfRequiredValues="1">
          <DiscreteInfo>
            <Structure>
              <Value Enum="e1" AdvanceLevel="1">a</Value>
              <CategoryInfo Combination="And">
                <Include Combination="Or">
                  <Cat>ec1</Cat>
                </Include>
              </CategoryInfo>
            </Structure>
            <Structure>
              <Value Enum="e2">b</Value>
              <CategoryInfo Combination="And">
                <Include Combination="Or">
                  <Cat>ec2</Cat>
                </Include>
              </CategoryInfo>
            </Structure>
            <Value Enum="e3" AdvanceLevel="1">c</Value>
          </DiscreteInfo>
        </String>
        <String Name="description" Label="text:"  NumberOfRequiredValues="1" MultipleLines="true">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>Enclosure Radiation</Cat>
              <Cat>Induction Heating</Cat>
            </Include>
          </CategoryInfo>
          <BriefDescription>Text added to top of input file</BriefDescription>
          <DefaultValue># Truchas simulation</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="numerics" Label="Numerics" BaseType=""  Unique="true">
      <CategoryInfo InheritanceMode="Or" Combination="And" />
      <ItemDefinitions>
        <Group Name="velocity-group" Label="Velocity"  NumberOfRequiredGroups="1" Extensible="true">
          <CategoryInfo InheritanceMode="Or" Combination="And" />
          <ComponentLabels CommonLabel="" />
          <ItemDefinitions>
            <Component Name="uniqueTest" Label="Unique Test"  EnforceCategories="false" Role="0" NumberOfRequiredValues="1">
              <CategoryInfo InheritanceMode="Or" Combination="And">
                <Include Combination="Or">
                  <Cat>Heat Transfer</Cat>
                </Include>
              </CategoryInfo>
              <Accepts>
                <Resource Name="smtk::model::Resource" Filter="face" />
              </Accepts>
              <Rejects />
            </Component>
            <Double Name="velocity-value" Label="Velocity"  NumberOfRequiredValues="4">
              <CategoryInfo InheritanceMode="Or" Combination="And">
                <Include Combination="Or">
                  <Cat>Fluid Flow</Cat>
                </Include>
              </CategoryInfo>
              <ComponentLabels>
                <Label>t</Label>
                <Label>u</Label>
                <Label>v</Label>
                <Label>w</Label>
              </ComponentLabels>
              <DefaultValue>0</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
        <Double Name="dt_init" Label="Dt_Init"  NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>Fluid Flow</Cat>
            </Include>
          </CategoryInfo>
          <BriefDescription>Integration time step value used for
the first computation cycle</BriefDescription>
          <DefaultValue>9.9999999999999995e-07</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_grow" Label="Dt_Grow"  NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>Fluid Flow</Cat>
            </Include>
          </CategoryInfo>
          <BriefDescription>A factor to multiply the current integration time step
when estimating the next cycle time step.</BriefDescription>
          <DefaultValue>1.05</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_max" Label="Dt_Max"  NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>Solid Mechanics</Cat>
            </Include>
          </CategoryInfo>
          <BriefDescription>Maximum allowable value for the time step.</BriefDescription>
          <DefaultValue>10</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="dt_min" Label="Dt_Min"  NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>Solid Mechanics</Cat>
            </Include>
          </CategoryInfo>
          <BriefDescription>Minimum allowable value for the time step.</BriefDescription>
          <DefaultValue>9.9999999999999995e-07</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="outputs" Label="Outputs" BaseType=""  Unique="true">
      <CategoryInfo InheritanceMode="Or" Combination="And" />
      <ItemDefinitions>
        <Double Name="start-time" Label="Start Time"  NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>Induction Heating</Cat>
              <Cat>Solid Mechanics</Cat>
            </Include>
          </CategoryInfo>
          <DefaultValue>0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="end-time" Label="End Time"  NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>Induction Heating</Cat>
              <Cat>Solid Mechanics</Cat>
            </Include>
          </CategoryInfo>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="output-dt" Label="Initial Output Delta-Time"  NumberOfRequiredValues="1">
          <CategoryInfo InheritanceMode="Or" Combination="And">
            <Include Combination="Or">
              <Cat>Heat Transfer</Cat>
            </Include>
          </CategoryInfo>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Group Name="output-times" Label="Additional Output Control"  NumberOfRequiredGroups="0" Extensible="true">
          <CategoryInfo InheritanceMode="Or" Combination="And" />
          <ComponentLabels CommonLabel="" />
          <ItemDefinitions>
            <Double Name="time" Label="Output Times"  NumberOfRequiredValues="2">
              <CategoryInfo InheritanceMode="Or" Combination="And">
                <Include Combination="Or">
                  <Cat>Fluid Flow</Cat>
                </Include>
              </CategoryInfo>
              <ComponentLabels>
                <Label>After time:</Label>
                <Label>Use delta time:</Label>
              </ComponentLabels>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="simulation-control" Label="Simulation Control" BaseType=""  Unique="true">
      <CategoryInfo InheritanceMode="Or" Combination="And" />
      <ItemDefinitions>
        <Group Name="simulation-control" Label="Simulation Control"  Optional="true" IsEnabledByDefault="false" NumberOfRequiredGroups="1">
          <CategoryInfo InheritanceMode="Or" Combination="And" />
          <BriefDescription>This item is superseded by the displacement sequence when
moving-enclosure radiation is enabled</BriefDescription>
          <ItemDefinitions>
            <Double Name="phase-start-times" Label="Phase Start Times"  NumberOfRequiredValues="1" Extensible="true">
              <CategoryInfo InheritanceMode="Or" Combination="And">
                <Include Combination="Or">
                  <Cat>Heat Transfer</Cat>
                </Include>
              </CategoryInfo>
              <BriefDescription>The list of starting times of each of the phases</BriefDescription>
            </Double>
            <Double Name="phase-init-dt-factor" Label="Phase Init Dt Factor"  NumberOfRequiredValues="1">
              <CategoryInfo InheritanceMode="Or" Combination="And">
                <Include Combination="Or">
                  <Cat>Fluid Flow</Cat>
                  <Cat>Heat Transfer</Cat>
                  <Cat>Solid Mechanics</Cat>
                </Include>
              </CategoryInfo>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <!--********** Workflow Views ***********-->
  <Views>
    <View Type="Group" Name="TopLevel" FilterByAdvanceLevel="true" TabPosition="North" TopLevel="true">
      <Views>
        <View Title="Configurations" />
        <View Title="Test" />
      </Views>
    </View>
    <View Type="Analysis" Name="Configurations" AnalysisAttributeName="analysis" AnalysisAttributeType="analysis" />
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
