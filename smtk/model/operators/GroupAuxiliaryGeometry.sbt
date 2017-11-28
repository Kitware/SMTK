<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "GroupAuxiliaryGeometry" Operator -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/NewOp.xml"/>
    <AttDef Type="group auxiliary geometry" BaseType="operator"
      Label="Model Entities - Group Auxiliary Geometry">
      <AssociationsDef Name="children" NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>aux_geom</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Create a new auxiliary geometry that groups the associated
        auxiliary geometries underneath it.
      </BriefDescription>
      <DetailedDescription>
        Create a new auxiliary geometry that groups the associated
        auxiliary geometries underneath it.

        This reparents the associated auxiliary geometries under the
        newly-created auxiliary geometry; although not-yet-implemented,
        they should inherit properties and transforms applied to their
        new parent, making it possible to create assemblies.
      </DetailedDescription>
      <ItemDefinitions>
        <String Name="name"
          NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>The name of the grouping auxiliary geometry.</BriefDescription>
          <DetailedDescription>
            The name of the auxiliary geometry created to group the existing, associated ones.

            When no name is specified (the default) then one will be generated automatically.
          </DetailedDescription>
          <DefaultValue></DefaultValue>
        </String>
        <Int Name="dimension" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>The dimension of the geometric point locus related to this entity.</BriefDescription>
          <DetailedDescription>
            The dimension of the geometric point locus related to this entity.

            If negative, then the dimension is unspecified and computed as the maximum of
            all the associated auxiliary geometries.
            The dimension affects the order in which rendering occurs (so that edges are drawn
            on top of coincident faces and so forth).
          </DetailedDescription>
          <DiscreteInfo DefaultIndex="4">
            <Value Enum="point">0</Value>
            <Value Enum="curve">1</Value>
            <Value Enum="surface">2</Value>
            <Value Enum="volume">3</Value>
            <Value Enum="mixed- or unknown-dimension">-1</Value>
          </DiscreteInfo>
        </Int>

        <Double Name="scale" NumberOfRequiredValues="3">
          <BriefDescription>
            Scale data read from the URL? If so, specify a scale factor for each axis.
          </BriefDescription>
          <DetailedDescription>
            Enabling this item allows you to specify a scale factor per axis for the
            auxiliary geometry.
            Scaling is performed about the origin before rotation and translation.
          </DetailedDescription>
          <DefaultValue>1, 1, 1</DefaultValue>
        </Double>
        <Double Name="rotate" NumberOfRequiredValues="3">
          <BriefDescription>
            Rotate data read from the URL? If so, specify angles about each axis in degrees.
          </BriefDescription>
          <DetailedDescription>
            Enabling this item allows you to specify angles (in degrees) about which to rotate
            the auxiliary geometry. Angles are specified about the origin and rotation is applied
            before translation.
          </DetailedDescription>
          <DefaultValue>0, 0, 0</DefaultValue>
        </Double>
        <Double Name="translate" NumberOfRequiredValues="3">
          <BriefDescription>Translate data read from the URL? If so, specify a vector.</BriefDescription>
          <DetailedDescription>
            Enabling this item allows you to specify a vector to add to each original point
            of the auxiliary geometry.
            Translation is applied after scaling and rotation;
            therefore the vector is not modified by the specifed scaling and rotation (if any).
          </DetailedDescription>
          <DefaultValue>0, 0, 0</DefaultValue>
        </Double>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(group auxiliary geometry)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
