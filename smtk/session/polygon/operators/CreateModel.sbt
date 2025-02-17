<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Polygon "CreateModel" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create model" Label="Planar Polygonal Model" BaseType="operation">
      <BriefDescription>Create a planar model.</BriefDescription>
      <DetailedDescription>
        Create a model given a set of coordinate axes in 3D and a minimum feature size.
      </DetailedDescription>

      <!-- A polygon model can be created within an existing
           resource (or an existing resource's session) if one is
           provided. Otherwise, a new resource is created -->
      <AssociationsDef NumberOfRequiredValues="0" Extensible="true"
                       MaxNumberOfValues="1" AdvanceLevel="1">
        <Accepts><Resource Name="smtk::session::polygon::Resource" Filter="model"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>

      <!-- In the event that we are creating a model within  an existing
           resource, this enumeration allows the user to select
           whether the model should simply use the resource's session
           or if the new model should be a part of the resource itself -->
        <String Name="session only" Label="session" AdvanceLevel="1">
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="this file">import into this file</Value>
            </Structure>
            <Structure>
              <Value Enum="this session">import into a new file using this file's session</Value>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="name" NumberOfValuesRequired="1" Optional="true">
          <BriefDescription>A user-assigned name for the model.</BriefDescription>
          <DetailedDescription>
            A user-assigned name for the model.
            The name need not be unique, but unique names are best.
            If not assigned, a machine-generated name will be assigned.
          </DetailedDescription>
        </String>
        <Int Name="construction method" AdvanceLevel="1">
          <ChildrenDefinitions>
            <Double Name="origin" NumberOfRequiredValues="3" Optional="true">
              <DefaultValue>0., 0., 0.</DefaultValue>
              <BriefDescription>The base point (origin) of the model in 3D world coordinates.</BriefDescription>
              <DetailedDescription>
                This vector specifies where the model's origin lies in 3D.
                The x axis and y axis properties specify the planar coordinate system eminating from the origin.
              </DetailedDescription>
            </Double>
            <Double Name="x axis" NumberOfRequiredValues="3" Optional="true">
              <DefaultValue>1., 0., 0.</DefaultValue>
              <BriefDescription>Direction and length of planar unit-length x-axis in world coordinates.</BriefDescription>
              <DetailedDescription>
                The direction along which x varies in the planar model.
                This vector may not be zero.

                The length of this vector is ignored when _feature size_ is specified.
                When _model scale_ is specified instead of _feature size_, then the length of this
                vector in 3D corresponds to _model scale_ units in the planar model.
              </DetailedDescription>
            </Double>
            <Double Name="y axis" NumberOfRequiredValues="3" Optional="true">
              <DefaultValue>0., 1., 0.</DefaultValue>
              <BriefDescription>Direction and length of planar unit-length y-axis in world coordinates.</BriefDescription>
              <DetailedDescription>
                The direction along which y varies in the planar model.
                This vector may not be zero.

                The length of this vector is ignored when _feature size_ is specified.
                When _model scale_ is specified instead of _feature size_, then the length of this
                vector in 3D corresponds to _model scale_ units in the planar model.
              </DetailedDescription>
            </Double>
            <Double Name="z axis" NumberOfRequiredValues="3" Optional="true">
              <DefaultValue>0., 0., 1.</DefaultValue>
              <BriefDescription>Normal to the model plane in world coordinates.</BriefDescription>
              <DetailedDescription>
                The direction perpendicular to the planar model.
                This vector may not be zero but need not be normalized.
              </DetailedDescription>
            </Double>
            <Double Name="feature size" NumberOfRequiredValues="1">
              <DefaultValue>1e-6</DefaultValue>
              <BriefDescription>The smallest resolvable edge length in world coordinates.</BriefDescription>
              <DetailedDescription>
                This is the smallest world-coordinate edge length that you wish
                resolved across all edges in a model.

                It is **not** a guarantee that vertices closer than this
                distance will be snapped together.
                It is **not** a guarantee that edges must always be longer than this.
                It **is** a guarantee that vertices further apart than the feature size
                and edges longer than the feature size will be properly resolved.

                This is not equivalent to a difference of 1 in the integer
                coordinate system used by the modeling session as then
                intersection points along short (but not feature-sized or
                smaller) lines would have unacceptable chord errors.
              </DetailedDescription>
            </Double>
            <Int Name="model scale" NumberOfRequiredValues="1">
              <DefaultValue>2310</DefaultValue>
              <BriefDescription>The denominator .</BriefDescription>
              <DetailedDescription>
                The length along which the associated entities should be swept.

                This parameter is optional.
                If unspecified or set to zero,
                the sweep distance is determined by the length of the
                "extrusion direction" item
                (or fails when the "extrusion direction" is ill-defined).
              </DetailedDescription>
            </Int>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <!-- Option 0: base point, x-axis, y-axis, and feature size -->
            <Structure>
              <Value Enum="origin, 2 axes, and feature size">0</Value>
              <Items>
                <Item>origin</Item>
                <Item>x axis</Item>
                <Item>y axis</Item>
                <Item>feature size</Item>
              </Items>
            </Structure>
            <!-- Option 1: base point, x-axis, y-axis, and feature size -->
            <Structure>
              <Value Enum="origin, normal, x axis, and feature size">1</Value>
              <Items>
                <Item>origin</Item>
                <Item>x axis</Item>
                <Item>z axis</Item>
                <Item>feature size</Item>
              </Items>
            </Structure>
            <!-- Option 2: base point, x-axis, y-axis, and integer divisor -->
            <Structure>
              <Value Enum="origin, 2 axes, and model scale">1</Value>
              <Items>
                <Item>origin</Item>
                <Item>x axis</Item>
                <Item>y axis</Item>
                <Item>model scale</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create model)" BaseType="result">
      <ItemDefinitions>
        <Component Name="model">
          <Accepts>
            <Resource Name="smtk::session::polygon::Resource" Filter=""/>
          </Accepts>
        </Component>

      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
    <View Type="Operation" Title="Model - Create"
          FilterByAdvanceLevel="true" UseSelectionManager="true">
      <InstancedAttributes>
        <Att Type="create model"/>
      </InstancedAttributes>
    </View>
  </Views>

</SMTK_AttributeResource>
