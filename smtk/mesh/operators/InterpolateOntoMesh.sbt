<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "interpolate onto mesh" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="interpolate onto mesh"
            Label="Mesh - Interpolate data onto mesh" BaseType="operator">
      <BriefDescription>
        Create a field on mesh nodes/elements from interpolated 3-dimensional data.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Create a field on mesh nodes/elements from interpolated 3-dimensional data.
        &lt;p&gt;This operator accepts as input three-dimensional points
        with associated scalar values; it interpolates these values
        onto either the points or the cells of the mesh using
        Shepard's method for interpolation. The input points can be
        inserted manually or read from a CSV file.
      </DetailedDescription>
      <ItemDefinitions>

        <String Name="input data" Label="Input Data">

          <BriefDescription>The input data (coordinates and scalar values) to use for interpolation.</BriefDescription>
          <DetailedDescription>
            An input data set is required to elevate a mesh. It can be
            an auxiliary geometry, an input file, or a collection of
            coordinates and scalar values.
          </DetailedDescription>

          <ChildrenDefinitions>
            <ModelEntity Name="auxiliary geometry" Label = "Auxiliary Geometry" NumberOfRequiredValues="1">
              <MembershipMask>aux_geom</MembershipMask>
              <BriefDescription>
                An external data set whose values determine the interpolated values.
              </BriefDescription>
            </ModelEntity>

            <File Name="ptsfile" Label="Input File" NumberOfValues="1"
                  ShouldExist="true" FileFilters="CSV file (*.csv);;Aux Geom Files (*.tif *.tiff *.dem *.vti *.vtp *.vtu *.vtm *.obj *.ply *.pts *.xyz);; Image files (*.tif *.tiff *.dem);;VTK files (*.vti *.vtp *.vtu *.vtm);;Wavefront OBJ files (*.obj);;Point Cloud Files (*.pts *.xyz);;Stanford Triangle Files (*.ply);;All files (*.*)">
              <BriefDescription>
                A file describing a data set whose values determine the interpolated values.
              </BriefDescription>
            </File>

            <Group Name="input filter" Label="Filter Input" AdvanceLevel="1">

              <BriefDescription>Input data filter options.</BriefDescription>

              <DetailedDescription>
                Options to filter input data prior to interpolation.
              </DetailedDescription>

              <ItemDefinitions>

                <Double Name="min threshold" Label="Minimum Threshold"  Optional="true"
                        IsEnabledByDefault="false" NumberOfRequiredValues="1" Extensible="false">
                  <BriefDescription>Input data less than this value will be ignored.</BriefDescription>
                </Double>

                <Double Name="max threshold" Label="Maximum Threshold"  Optional="true"
                        IsEnabledByDefault="false" NumberOfRequiredValues="1" Extensible="false">
                  <BriefDescription>Input data greater than this value will be ignored.</BriefDescription>
                </Double>

                <Void Name="invert scalars" Label="Invert Scalar Values"
                      Optional="true" IsEnabledByDefault="false">
                  <BriefDescription>This toggle adds a prefactor of -1 to the values in the external data set prior to averaging.</BriefDescription>
                </Void>

              </ItemDefinitions>

            </Group>

            <Group Name="points" Label="Interpolation Points"
                   Extensible="true" NumberOfRequiredGroups="1" >
              <BriefDescription>The points and their scalar values to interpolate.</BriefDescription>
              <DetailedDescription>
                An implicit function is described by interpolating points
                and their associated scalar value.
              </DetailedDescription>
              <ItemDefinitions>
                <Double Name="point" Label="Point" NumberOfRequiredValues="4">
                  <ComponentLabels>
                    <Label>X</Label>
                    <Label>Y</Label>
                    <Label>Z</Label>
                    <Label>Scalar</Label>
                  </ComponentLabels>
                </Double>
              </ItemDefinitions>
            </Group>
          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="Auxiliary Geometry">auxiliary geometry</Value>
	      <Items>
		<Item>auxiliary geometry</Item>
		<Item>input filter</Item>
	      </Items>
	    </Structure>
	    <Structure>
              <Value Enum="Input File">ptsfile</Value>
	      <Items>
	        <Item>ptsfile</Item>
		<Item>input filter</Item>
	      </Items>
	    </Structure>
	    <Structure>
              <Value Enum="Interpolation Points">points</Value>
	      <Items>
	        <Item>points</Item>
	      </Items>
	    </Structure>
          </DiscreteInfo>
        </String>

        <MeshEntity Name="mesh" Label="Mesh" NumberOfRequiredValues="1" Extensible="true" >
          <BriefDescription>The mesh onto which the data is interpolated.</BriefDescription>
        </MeshEntity>

        <String Name="interpolation scheme" Label="Interpolation Scheme">

          <BriefDescription>The interpolation scheme to apply to the input data</BriefDescription>
          <DetailedDescription>
            The scalar values from the input data must be interpolated
            onto the nodes of the mesh. Currently, the supported
            techniques are Radial Average (unweighted average of all
            points within a cylinder of a given radius from a mesh
            node) and Inverse Distance Weighting (weighted average of
            all the points in the data set according to the distance
            between the datapoint and the mesh node).
          </DetailedDescription>

          <ChildrenDefinitions>

        <Double Name="radius" Label="Radius for Averaging Values" NumberOfRequiredValues="1" Extensible="false">
          <BriefDescription>The radius used to identify a point locus for computing values.</BriefDescription>
          <DetailedDescription>
            The radius used to identify a point locus for computing
            values.

            For each node in the input mesh, a corresponding point
            locus whose positions projected onto the x-y plane are
            within a radius of the node is used to compute a new
            svalue.
          </DetailedDescription>
        </Double>

        <String Name="external point values" Label="External Points:">

          <BriefDescription>Treatment for points in the mesh that fall outside of the range of the dataset.</BriefDescription>

          <ChildrenDefinitions>
            <Double Name="external point value" Label="External Point Value" NumberOfRequiredValues="1" Extensible="false">
          <BriefDescription>The z-coordinate of every point outside of the data set will be assigned this value</BriefDescription>
          <DefaultValue>0.</DefaultValue>
        </Double>

          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="Left Unchanged">left unchanged</Value>
	    </Structure>
	    <Structure>
              <Value Enum="Set to Value">set to value</Value>
	      <Items>
	        <Item>external point value</Item>
	      </Items>
	    </Structure>
	    <Structure>
              <Value Enum="Set to NaN">set to NaN</Value>
	    </Structure>
          </DiscreteInfo>

        </String>

        <Double Name="power" Label="Weighting Power" NumberOfRequiredValues="1" Extensible="no">
          <BriefDescription>The weighting power used to interpolate source points.</BriefDescription>
          <DetailedDescription>
            The weighting power used to interpolate source points.

            The inverse distance value is exponentiated by
            this unitless value when computing weighting coefficients.
          </DetailedDescription>
          <DefaultValue>1.</DefaultValue>
        </Double>

          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
	    <Structure>
              <Value Enum="Radial Average">radial average</Value>
	      <Items>
		<Item>radius</Item>
		<Item>external point values</Item>
	      </Items>
	    </Structure>
	    <Structure>
              <Value Enum="Inverse Distance Weighting">inverse distance weighting</Value>
	      <Items>
	        <Item>power</Item>
	      </Items>
	    </Structure>
          </DiscreteInfo>

        </String>

        <Group Name="output filter" Label="Clamp Output to Range" AdvanceLevel="1">

          <BriefDescription>Ignore input data that falls outside of a given range.</BriefDescription>

          <DetailedDescription>
            A range (minimum and maximum value) for input scalar
            data. Input data values that fall outside of this
            range will be omitted from the interpolation calculation.
          </DetailedDescription>

          <ItemDefinitions>

            <Double Name="min value" Label="Minimum Value"
                    NumberOfRequiredValues="1" Extensible="false"
                    Optional="true">
              <BriefDescription>Lower limit to the resulting mesh's value range.</BriefDescription>
              <DefaultValue>0.0</DefaultValue>
            </Double>

            <Double Name="max value" Label="Maximum Value"
                    NumberOfRequiredValues="1" Extensible="false"
                    Optional="true">
              <BriefDescription>Upper limit to the resulting mesh's value range.</BriefDescription>
              <DefaultValue>0.0</DefaultValue>
            </Double>

          </ItemDefinitions>

        </Group>

        <String Name="dsname" Label="Field Name" NumberOfRequiredValues="1">
          <BriefDescription>The name of the generated data set.</BriefDescription>
        </String>

        <Int Name="interpmode" Label="Output Field Type" NumberOfRequiredValues="1" Extensible="false">
          <BriefDescription>Interpolate the data as cell-centered or point-centered on the mesh.</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Cell Fields">0</Value>
            <Value Enum="Point Fields">1</Value>
          </DiscreteInfo>
        </Int>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(interpolate onto mesh)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0"
                     Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Operator" Title="Interpolate onto Mesh" FilterByAdvanceLevel="true" UseSelectionManager="true">
      <InstancedAttributes>
        <Att Type="interpolate onto mesh" Name="interpolate onto mesh"/>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeSystem>
