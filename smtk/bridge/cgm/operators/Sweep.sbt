<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Sweep" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="sweep" BaseType="operator">
      <BriefDescription>Sweep the entities along the path or direction to form a body.</BriefDescription>
      <DetailedDescription>
        Create a wire or solid body given a set of curves and/or surfaces plus
        either a direction to sweep or a set of curves to sweep along.
        A draft angle may also be specified.
      </DetailedDescription>
      <AssociationsDef Name="workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>face|edge|vertex</MembershipMask>
        <BriefDescription>A set of points, curves, or surfaces to sweep.</BriefDescription>
        <DetailedDescription>
          A set of points, curves, or surfaces to sweep.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Int Name="construction method">
          <ChildrenDefinitions>
            <!-- Option 1: direction, distance, draft angle, and draft type -->
            <Double Name="extrusion direction" NumberOfRequiredValues="3" Optional="true">
              <DefaultValue>0., 0., 1.</DefaultValue>
              <BriefDescription>Direction along which the associated entities should be swept.</BriefDescription>
              <DetailedDescription>
                The direction (and optionally length) along which the associated entities should be swept.

                This parameter is optional.
                If unspecified or set to the zero vector,
                the sweep is perpendicular to the associated entities
                (or fails when a perpendicular direction is ill-defined)
                and the distance the items are swept is specified by the
                "sweep distance" item.
              </DetailedDescription>
            </Double>
            <Double Name="sweep distance" NumberOfRequiredValues="1">
              <DefaultValue>1.</DefaultValue>
              <BriefDescription>Distance along which the associated entities should be swept.</BriefDescription>
              <DetailedDescription>
                The length along which the associated entities should be swept.

                This parameter is optional.
                If unspecified or set to zero,
                the sweep distance is determined by the length of the
                "extrusion direction" item
                (or fails when the "extrusion direction" is ill-defined).
              </DetailedDescription>
            </Double>
            <Double Name="draft angle" NumberOfRequiredValues="1">
              <DefaultValue>0.</DefaultValue>
              <BriefDescription>The draft angle (in degrees).</BriefDescription>
            </Double>
            <Int Name="draft type" NumberOfRequiredValues="1">
              <BriefDescription>The type of corners created for positive draft angles.</BriefDescription>
              <ChildrenDefinitions/>
              <DiscreteInfo DefaultIndex="0">
                <!-- Values from CGM's GeometryType enum in util/GeometryDefines.h -->
                <Structure>
                  <Value Enum="sharp corner">1</Value>
                  <Items/>
                </Structure>
                <Structure>
                  <Value Enum="rounded corner">2</Value>
                  <Items/>
                </Structure>
              </DiscreteInfo>
            </Int>
            <!-- Option 2: rotational point, rotation axis, and angle -->
            <Double Name="axis base point" NumberOfRequiredValues="3">
              <DefaultValue>0., 0., 0.</DefaultValue>
              <BriefDescription>A point that the axis of revolution passes through.</BriefDescription>
              <DetailedDescription>
                The base point which, together with a direction vector,
                specifies the axis of revolution.
              </DetailedDescription>
            </Double>
            <Double Name="axis of revolution" NumberOfRequiredValues="3" Optional="true">
              <DefaultValue>0., 0., 1.</DefaultValue>
              <BriefDescription>Direction of the axis of revolution.</BriefDescription>
              <DetailedDescription>
                This vector, together with the base point, specifies the axis of revolution.
                It may not be zero but need not be unit length; only its direction is used.

                The direction of this vector also affects how the angle of the sweep should
                be interpreted: the right-hand rule determines the direction of
                positive values for the sweep angle.
              </DetailedDescription>
            </Double>
            <Double Name="sweep angle" NumberOfRequiredValues="1">
              <Min Inclusive="true">-360.0</Min>
              <Max Inclusive="true">360.</Max>
              <DefaultValue>360.</DefaultValue>
              <BriefDescription>The angle (in degrees) through which the associated entities should be revolved.</BriefDescription>
            </Double>
            <!-- Option 3: axis base point, axis of revolution, pitch, helix angle, and handedness -->
            <Double Name="helix angle" NumberOfRequiredValues="1">
              <DefaultValue>360.</DefaultValue>
              <BriefDescription>The angle (in degrees) through which the associated entities should be revolved.</BriefDescription>
            </Double>
            <Double Name="pitch" NumberOfRequiredValues="1">
              <DefaultValue>1.</DefaultValue>
              <BriefDescription>The distance between corresponding points measured along the axis of the sweep.</BriefDescription>
            </Double>
            <Int Name="handedness" NumberOfRequiredValues="1">
              <BriefDescription>Should the helix be right- or left-handed?</BriefDescription>
              <DetailedDescription>
                A helix translates geometry along the direction of the axis of revolution while
                at the same time rotating it tangent to this axis.
                A right-handed helix performs a positive translation (i.e., codirectional with the axis)
                while rotating in a direction given by aligning a right thumb with the axis and curling
                the fingers toward the thumb.
                A left-handed helix rotates the opposite direction for the same positive translation.
              </DetailedDescription>
              <ChildrenDefinitions/>
              <DiscreteInfo DefaultIndex="1">
                <!-- Values from CGM's GeometryType enum in util/GeometryDefines.h -->
                <Structure>
                  <Value Enum="left-handed">0</Value>
                  <Items/>
                </Structure>
                <Structure>
                  <Value Enum="right-handed">1</Value>
                  <Items/>
                </Structure>
              </DiscreteInfo>
            </Int>
            <!-- Option 4: curves (sweep path) -->
            <ModelEntity Name="sweep path" NumberOfRequiredValues="1" Extensible="true">
              <MembershipMask>edge</MembershipMask>
              <BriefDescription>The curve along which to sweep the associated entities.</BriefDescription>
              <DetailedDescription>
                The curve along which to sweep the associated entities.
                If multiple curves are specified, they should form a single piecewise-continuous curve
                with shared endpoints.
              </DetailedDescription>
            </ModelEntity>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="extrude">0</Value>
              <Items>
                <Item>extrusion direction</Item>
                <Item>sweep distance</Item>
                <Item>draft angle</Item>
                <Item>draft type</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="revolve">1</Value>
              <Items>
                <Item>axis base point</Item>
                <Item>axis of revolution</Item>
                <Item>sweep angle</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="sweep along curve(s)">2</Value>
              <Items>
                <Item>sweep path</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>
        <Int Name="keep inputs" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <BriefDescription>Should the inputs be copied before sweep?</BriefDescription>
          <DetailedDescription>
            If true, then copies of the workpieces are swept and the
            input models are untouched while their sweeps are added
            as new models in the session.
            Otherwise, the associated entities and (if specified) sweep path
            are consumed by the operation (this is the default).
          </DetailedDescription>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(sweep)" BaseType="result">
      <ItemDefinitions>
        <!-- The swept body (or bodies) is returned in the base result's "entities" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
