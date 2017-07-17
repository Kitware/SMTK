<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "CreateInstances" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create instances" BaseType="operator" Label="Model Entities - Create Instances">
      <AssociationsDef Name="entities" NumberOfRequiredValues="1">
        <MembershipMask>cell|aux_geom</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Create an instance (i.e., a transformed copy) of a model entity.
      </BriefDescription>
      <DetailedDescription>
        Create a transformed copy of any model cell or auxiliary geometry entity
        which has a tessellation.
        The instance does not consume resources the same way its prototype does;
        it uses the topological model and tessellation of its prototype instead.
      </DetailedDescription>
      <ItemDefinitions>
        <!-- TODO: The placement rule should be a separate attribute so that
             additional rules can be added as desired. -->
        <String Name="placement rule" NumberOfRequiredValues="1">
          <ChildrenDefinitions>
            <Group Name="placements" Extensible="true" NumberOfRequiredGroups="1">
              <ItemDefinitions>
                <Double Name="coordinates" NumberOfRequiredValues="3">
                  <BriefDescription>
                    The distance along each axis to translate the prototype when rendering the instance.
                  </BriefDescription>
                </Double>
              </ItemDefinitions>
            </Group>
            <Group Name="volume of interest" NumberOfRequiredGroups="3">
              <BriefDescription>
                Specify a box with minimum and maximum values along each axis.
              </BriefDescription>
              <ItemDefinitions>
                <Double Name="axis range" NumberOfRequiredValues="2">
                  <BriefDescription>
                    The range of values along a single axis.
                  </BriefDescription>
                </Double>
              </ItemDefinitions>
            </Group>
            <Int Name="sample size" NumberOfRequiredValues="1">
              <BriefDescription>
                The number of instance placement points to generate.
              </BriefDescription>
              <Min>1</Min>
            </Int>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <!-- Option 0: an explicit table of instance placements -->
            <Structure>
              <Value Enum="tabular">tabular</Value>
              <Items>
                <Item>placements</Item>
              </Items>
            </Structure>
            <!-- Option 1: uniform random placements inside a volume of interest (VOI) -->
            <Structure>
              <Value Enum="uniform random">uniform random</Value>
              <Items>
                <Item>volume of interest</Item>
                <Item>sample size</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
        <ModelEntity Name="snap to entity"
          Optional="true" IsEnabledByDefault="true"
          NumberOfRequiredValues="1" Extensible="true">
          <MembershipMask>cell|aux_geom</MembershipMask>
          <BriefDescription>
            If enabled, instance placements will be snapped to the nearest
            point on the tessellation of the given entities.
          </BriefDescription>
        </ModelEntity>

        <!-- TODO: Add support for masking placements -->
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create instances)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0" Extensible="true"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
