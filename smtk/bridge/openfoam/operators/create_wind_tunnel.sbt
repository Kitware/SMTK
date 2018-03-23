<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the OpenFOAM "create_wind_tunnel" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create wind tunnel" Label="Model - Create Wind Tunnel" BaseType="operator">
      <BriefDescription>
        Create a wind tunnel for OpenFOAM
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Create a wind tunnel for OpenFOAM.
        &lt;p&gt;This operator accepts as input the parameters
        for an axis-aligned background mesh. It uses
        OpenFOAM's blockMesh to construct a wind
        tunnel background mesh.
      </DetailedDescription>
      <ItemDefinitions>

        <Double Name="x dimensions" Label="X Dimensions" NumberOfRequiredValues="2">
          <ComponentLabels>
            <Label>min</Label>
            <Label>max</Label>
          </ComponentLabels>
          <BriefDescription>X coordinate values for an axis-aligned
          Cartesian background mesh</BriefDescription>
          <DefaultValue>-5,15</DefaultValue>
        </Double>

        <Double Name="y dimensions" Label="Y Dimensions" NumberOfRequiredValues="2">
          <ComponentLabels>
            <Label>min</Label>
            <Label>max</Label>
          </ComponentLabels>
          <BriefDescription>Y coordinate values for an axis-aligned
          Cartesian background mesh</BriefDescription>
          <DefaultValue>-4,4</DefaultValue>
        </Double>

        <Double Name="z dimensions" Label="Z Dimensions" NumberOfRequiredValues="2">
          <ComponentLabels>
            <Label>min</Label>
            <Label>max</Label>
          </ComponentLabels>
          <BriefDescription>Z coordinate values for an axis-aligned
          Cartesian background mesh</BriefDescription>
          <DefaultValue>0,8</DefaultValue>
        </Double>

        <Double Name="scaling factor" Label="Scaling Factor"
                AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>Scaling factor for the vertex coordinates</BriefDescription>
          <DefaultValue>1.</DefaultValue>
        </Double>

        <Int Name="number of cells" Label="Number of Cells" NumberOfRequiredValues="3">
          <ComponentLabels>
            <Label>X</Label>
            <Label>Y</Label>
            <Label>Z</Label>
          </ComponentLabels>
              <BriefDescription>Number of cells in each direction</BriefDescription>
          <DefaultValue>20,8,8</DefaultValue>
        </Int>

        <Int Name="expansion ratio" Label="Expansion Ratio"
             AdvanceLevel="1" NumberOfRequiredValues="3">
          <ComponentLabels>
            <Label>X</Label>
            <Label>Y</Label>
            <Label>Z</Label>
          </ComponentLabels>
          <BriefDescription>The expansion ratio enables the mesh to be graded, or refined, in specified directions. </BriefDescription>
          <DefaultValue>1,1,1</DefaultValue>
        </Int>

        <String Name="wind direction" Label="Wind Direction">

          <BriefDescription>Direction of wind flowing through the tunnel</BriefDescription>

          <DetailedDescription>
            Direction of wind flowing through the tunnel.

            The walls of the wind tunnel will have the labels "inlet" and
            "outlet" corresponding to this chosen direction. The
            remaining walls will have the label "wall".
          </DetailedDescription>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="From -X to +X">from -x to +x</Value>
	    </Structure>
	    <Structure>
              <Value Enum="From +X to -X">from +x to -x</Value>
	    </Structure>
	    <Structure>
              <Value Enum="From -Y to +Y">from -y to +y</Value>
	    </Structure>
	    <Structure>
              <Value Enum="From +Y to -Y">from +y to -y</Value>
	    </Structure>
          </DiscreteInfo>

        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create wind tunnel)" BaseType="result">
      <ItemDefinitions>
        <!-- The created wind tunnel. -->
        <ModelEntity Name="model" NumberOfRequiredValues="1" Extensible="1" MembershipMask="4096"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
