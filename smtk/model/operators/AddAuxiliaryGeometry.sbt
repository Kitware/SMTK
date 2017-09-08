<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "AddAuxiliaryGeometry" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="add auxiliary geometry" BaseType="operator" Label="Model - Add Auxiliary Geometry">
      <AssociationsDef Name="entities" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Add auxiliary geometry (scene geometry not part of the model domain)
        to a model or to another auxiliary geometry instance.
      </BriefDescription>
      <DetailedDescription>
        Add auxiliary geometry (scene geometry not part of the model domain)
        to a model or to another auxiliary geometry instance.

        Auxiliary geometry may be made hierarchical, by adding an auxiliary
        geometry instance with no URL to a model and then adding 1 or more
        auxiliary geometry instances to the first instance above.
        This is useful when you wish to instance multiple files into the
        scene as if they were a single file.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="url" Label = "Filename"  Optional="true" NumberOfRequiredValues="1" IsEnabledByDefault="true" ShouldExist="true"
          FileFilters="Aux Geom Files (*.tif *.tiff *.dem *.vti *.vtp *.vtu *.vtm *.obj *.ply *.pts *.xyz);; Image files (*.tif *.tiff *.dem);;VTK files (*.vti *.vtp *.vtu *.vtm);;Wavefront OBJ files (*.obj);;Point Cloud Files (*.pts *.xyz);;Stanford Triangle Files (*.ply);;All files (*.*)">
          <BriefDescription>The file containing the auxiliary scene geometry.</BriefDescription>
        </File>
        <String Name="type" NumberOfRequiredValues="1" AdvanceLevel="1">
          <BriefDescription>The type of data at the specified URL.</BriefDescription>
          <DetailedDescription>
            The type of data at the specified URL.
            The default value is an empty string, which indicates that the type
            is to be inferred from the URL.
            Valid vales are currently: "pts", "xyz" vtp", "vtu", "vti", "tif", "dem", and "vtm".
          </DetailedDescription>
          <DefaultValue></DefaultValue>
        </String>
        <Int Name="dimension" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>The dimension of the geometric point locus related to this entity.</BriefDescription>
          <DetailedDescription>
            The dimension of the geometric point locus related to this entity.

            If negative, then the dimension is unspecified and assumed either
            to be unknown or to consist of multiple geometric primitives of different dimension.
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

        <Void Name="separate representation" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false"
          Label="Display as separate representation from model">
          <BriefDescription>
            Should the auxiliary geometry's representation be separate from its owning model's?
          </BriefDescription>
          <DetailedDescription>
            Should the auxiliary geometry's representation be separate from its owning model's?
            If yes, a separate rendering pipeline will be created for auxiliary geometry and
            its representation will be controlled with its own set of display properties;
            if no, the geometry will be shown and controlled as sub-blocks in the model's multiblock dataset,
            which may be less flexible.
          </DetailedDescription>
        </Void>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(add auxiliary geometry)" BaseType="result">
      <ItemDefinitions>
        <!-- The modified entities are stored in the base result's "modified" item. -->
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0" Extensible="true"/>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
